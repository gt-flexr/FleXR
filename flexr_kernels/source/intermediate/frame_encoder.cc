#ifdef __FLEXR_KERNEL_FRAME_ENCODER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    /* Constructor() */
    FrameEncoder::FrameEncoder(std::string id, std::string encoderName, int width, int height, int bitrate, int fps):
      FleXRKernel(id), encoderName(encoderName), width(width), height(height)
    {
      setName("FrameEncoder");
      portManager.registerInPortTag ("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerOutPortTag("out_encoded_frame", utils::sendLocalPointerMsgCopy, utils::serializeEncodedFrame);

      // Encoder
      av_register_all();
      avcodec_register_all();
      encoder = avcodec_find_encoder_by_name(encoderName.c_str());
      if(!(encoder)) {
        debug_print("encoder %s is not found", encoderName.c_str());
        exit(1);
      }

      encoderContext               = avcodec_alloc_context3(encoder);
      encoderContext->width        = width;
      encoderContext->height       = height;
      encoderContext->bit_rate     = bitrate;
      encoderContext->pix_fmt      = AV_PIX_FMT_YUV420P;
      encoderContext->time_base    = AVRational{1, fps};
      encoderContext->framerate    = AVRational{fps, 1};
      encoderContext->delay        = 0;        // https://bit.ly/2NlAtEl
      encoderContext->gop_size     = 12;       // https://bit.ly/2NtOGit
      encoderContext->max_b_frames = 0;        // https://bit.ly/3pIMUal

      if (strcmp(encoderContext->codec->name, "libx264") == 0)
      {
        av_opt_set(encoderContext->priv_data, "preset", "ultrafast", 0);
        av_opt_set(encoderContext->priv_data, "tune", "zerolatency", 0);
        av_opt_set(encoderContext->priv_data, "vsink", "0", 0);
        av_opt_set(encoderContext->priv_data, "qp", "0", 0);
      }

      if (strcmp(encoderContext->codec->name, "h264_nvenc") == 0 ||
          strcmp(encoderContext->codec->name, "nvenc_h264") == 0 )
      {
        av_opt_set(encoderContext->priv_data, "preset", "fast", 0);
        av_opt_set(encoderContext->priv_data, "delay", 0, 0);
        av_opt_set(encoderContext->priv_data, "vsink", "0", 0);
        av_opt_set(encoderContext->priv_data, "qp", "0", 0);
        //av_opt_set(encoderContext->priv_data, "zerolatency", "true", 0);
        //av_opt_set(encCtx->priv_data, "2pass", "true", 0);
        //av_opt_set_int(encCtx->priv_data, "level", 51, AV_OPT_SEARCH_CHILDREN);
      }

      if (strcmp(encoderContext->codec->name, "mjpeg") == 0) {
        debug_print("mjpeg Setting");
        encoderContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
        av_opt_set(encoderContext->priv_data, "qscale", "2", 0);
      }

      int ret = avcodec_open2(encoderContext, encoder, NULL);
      if(ret < 0) {
        debug_print("encoder %s open failed.", encoderName.c_str());
        exit(1);
      }

      /* Encoding Frame Init */
      encodingFrame = av_frame_alloc();
      encodingFrame->width = width; encodingFrame->height = height;
      encodingFrame->format = encoderContext->pix_fmt;

      av_image_fill_arrays(encodingFrame->data, encodingFrame->linesize, NULL,
                           static_cast<AVPixelFormat>(encodingFrame->format),
                           encodingFrame->width, encodingFrame->height, 1);

      yuvFrame = cv::Mat::zeros(height*1.5, width, CV_8UC1);
    }


    /* Destructor() */
    FrameEncoder::~FrameEncoder()
    {
      avcodec_close(encoderContext);
      av_frame_free(&encodingFrame);
    }


    /* Run() */
    raft::kstatus FrameEncoder::run()
    {
      EncoderOutEncodedFrameType *outEncodedFrame = portManager.getOutputPlaceholder<EncoderOutEncodedFrameType>("out_encoded_frame");

      AVPacket encodingPacket;
      av_init_packet(&encodingPacket);

      EncoderInFrameType *inFrame = portManager.getInput<EncoderInFrameType>("in_frame");
      if(inFrame->data.rows != (size_t)height || inFrame->data.cols != (size_t)width) {
        debug_print("inMat size is not compatible.");
        inFrame->data.release();
        portManager.freeInput("in_frame", inFrame);
        return raft::proceed;
      }
      inFrame->printHeader();

      double st = getTsNow();

      cv::cvtColor(inFrame->data.useAsCVMat(), yuvFrame, cv::COLOR_RGB2YUV_YV12);
      av_image_fill_arrays(encodingFrame->data, encodingFrame->linesize, yuvFrame.data,
                           static_cast<AVPixelFormat>(encodingFrame->format),
                           encodingFrame->width, encodingFrame->height, 1);

      int ret = avcodec_send_frame(encoderContext, encodingFrame);
      while (ret >= 0)
      {
        ret = avcodec_receive_packet(encoderContext, &encodingPacket);
        if(ret == 0)
        {
          double enct = getTsNow();
          outEncodedFrame->data     = encodingPacket.data;
          outEncodedFrame->setHeader(inFrame->tag, inFrame->seq, inFrame->ts, encodingPacket.size);
          outEncodedFrame->printHeader();

          portManager.sendOutput("out_encoded_frame", outEncodedFrame);
          double et = getTsNow();
          debug_print("encodeTime(%lf), sentSize(%d)", et-st, outEncodedFrame->dataSize);
          if(logger.isSet()) logger.getInstance()->info("encodingTime/rtpSendingTime/KernelExeTime/Sent Size\t{}\t {}\t {}\t {}",
              enct-st, et-enct, et-st, outEncodedFrame->dataSize);
          break;
        }
        else
        {
          std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        av_packet_unref(&encodingPacket); // deleted?
      }

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

