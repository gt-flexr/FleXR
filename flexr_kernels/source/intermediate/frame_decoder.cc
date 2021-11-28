#ifdef __FLEXR_KERNEL_FRAME_DECODER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    /* Constructor() */
    FrameDecoder::FrameDecoder(std::string id, std::string decoderName, int width, int height):
      width(width), height(height), decoderName(decoderName),
      FleXRKernel(id)
    {
      setName("FrameDecoder");
      portManager.registerInPortTag ("in_encoded_frame", components::BLOCKING, utils::deserializeEncodedFrame);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, utils::serializeRawFrame);

      // Decoder
      av_register_all();
      avcodec_register_all();

      decoder = avcodec_find_decoder_by_name(decoderName.c_str());
      if(!(decoder)) {
        debug_print("decoder %s is not found", decoderName.c_str());
        exit(1);
      }

      decoderContext = avcodec_alloc_context3(decoder);
      decoderContext->max_b_frames = 0;
      decoderContext->time_base = AVRational{1, 60};
      decoderContext->framerate = AVRational{60, 1};

      int ret = avcodec_open2(decoderContext, decoder, NULL);
      if(ret < 0) {
        debug_print("decoder %s open failed.", decoderName.c_str());
        exit(1);
      }

      AVPixelFormat fmt = AV_PIX_FMT_YUV420P;
      if (strcmp(decoderName.c_str(), "h264") == 0 ||
          strcmp(decoderName.c_str(), "h264_cuvid")) {
        fmt = AV_PIX_FMT_YUV420P;
      }
      if (strcmp(decoderName.c_str(), "mjpeg") == 0) {
        fmt = AV_PIX_FMT_YUVJ420P;
      }


      decodingFrame = av_frame_alloc();
      decodingFrame->width = width; decodingFrame->height = height;
      decodingFrame->format = fmt;
      decodingFrameSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);;
      decodingFrameBuffer = (uint8_t*)av_malloc(decodingFrameSize);

      av_image_fill_arrays(decodingFrame->data, decodingFrame->linesize, decodingFrameBuffer, fmt,
                           width, height, 1);

      yuvFrame = cv::Mat::zeros(height*1.5, width, CV_8UC1);
    }


    /* Destructor() */
    FrameDecoder::~FrameDecoder()
    {
      av_frame_free(&decodingFrame);
      avcodec_close(decoderContext);
    }


    /* Run() */
    raft::kstatus FrameDecoder::run()
    {
      DecoderOutFrameType *outFrame = portManager.getOutputPlaceholder<DecoderOutFrameType>("out_frame");

      outFrame->data = types::Frame(height, width, CV_8UC3);
      AVPacket decodingPacket;
      int ret = 0;



      DecoderInEncodedFrameType *inEncodedFrame = portManager.getInput<DecoderInEncodedFrameType>("in_encoded_frame");
      outFrame->setHeader(inEncodedFrame->tag, inEncodedFrame->seq, inEncodedFrame->ts, inEncodedFrame->dataSize);
      outFrame->printHeader();

      double st = getTsNow();
      av_packet_from_data(&decodingPacket, inEncodedFrame->data, inEncodedFrame->dataSize);
      decodingPacket.side_data_elems = 0;

      ret = avcodec_send_packet(decoderContext, &decodingPacket);

      while (ret >= 0) {
        ret = avcodec_receive_frame(decoderContext, decodingFrame);

        if(ret == 0) {
          av_image_copy_to_buffer(yuvFrame.data, yuvFrame.total(), decodingFrame->data, decodingFrame->linesize,
                                  static_cast<AVPixelFormat>(decodingFrame->format),
                                  decodingFrame->width, decodingFrame->height, 1);
          if(decoderName == "h264")
            cv::cvtColor(yuvFrame, outFrame->data.useAsCVMat(), cv::COLOR_YUV420p2RGB);
          else if(decoderName == "h264_cuvid")
            cv::cvtColor(yuvFrame, outFrame->data.useAsCVMat(), cv::COLOR_YUV2BGR_NV12);
          else if(decoderName == "mjpeg")
            cv::cvtColor(yuvFrame, outFrame->data.useAsCVMat(), cv::COLOR_YUV2RGB);
          outFrame->dataSize = outFrame->data.useAsCVMat().total() * outFrame->data.useAsCVMat().elemSize();

          double et = getTsNow();
          debug_print("decodeTime(%lf), recvInfo %s:%d size(%d)", et-st, outFrame->tag, outFrame->seq, inEncodedFrame->dataSize);
          if(logger.isSet()) logger.getInstance()->info("st/et/decodingTime/recvSize\t{}\t {}\t {}\t {}",
              st, et, et-st, inEncodedFrame->dataSize);

          portManager.sendOutput("out_frame", outFrame);
          break;
        }
      }

      delete inEncodedFrame->data;
      portManager.freeInput("in_encoded_frame", inEncodedFrame);

      freqManager.adjust();
      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

#endif

