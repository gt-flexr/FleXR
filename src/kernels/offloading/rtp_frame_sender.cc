#include "kernels/offloading/rtp_frame_sender.h"
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor() */
    RTPFrameSender::RTPFrameSender(std::string destAddr, int destPortBase, std::string encoderName,
                                   int width, int height, int bitrate, int fps):
      rtpSender(destAddr, destPortBase),
      encoderName(encoderName), width(width), height(height)
    {
      addInputPort<mxre::types::Frame>("in_frame");

      // Encoder
      av_register_all();
      avcodec_register_all();
      AVRational dstFps = {1, fps};
      encoder = avcodec_find_encoder_by_name(encoderName.c_str());
      if(!(encoder)) {
        debug_print("encoder %s is not found", encoderName.c_str());
        exit(1);
      }

      encoderContext = avcodec_alloc_context3(encoder); // encoder context
      encoderContext->width = width;
      encoderContext->height = height;
      encoderContext->bit_rate = bitrate;
      encoderContext->pix_fmt = AV_PIX_FMT_YUV420P;
      encoderContext->time_base = dstFps;
      encoderContext->framerate = av_inv_q(dstFps);
      encoderContext->delay = 0;        // https://bit.ly/2NlAtEl
      encoderContext->gop_size = 12;     // https://bit.ly/2NtOGit
      encoderContext->max_b_frames = 0; // https://bit.ly/3pIMUal
      if (strcmp(encoderContext->codec->name, "libx264") == 0)
      {
        av_opt_set(encoderContext->priv_data, "preset", "ultrafast", 0);
        av_opt_set(encoderContext->priv_data, "tune", "zerolatency", 0);
        av_opt_set(encoderContext->priv_data, "vsink", "0", 0);
      }
      if (strcmp(encoderContext->codec->name, "h264_nvenc") == 0 ||
          strcmp(encoderContext->codec->name, "nvenc_h264") == 0 )
      {
        av_opt_set(encoderContext->priv_data, "preset", "ll", 0);
        av_opt_set(encoderContext->priv_data, "zerolatency", "true", 0);
        av_opt_set(encoderContext->priv_data, "delay", 0, 0);
        av_opt_set(encoderContext->priv_data, "2pass", "false", 0);
        av_opt_set(encoderContext->priv_data, "vsink", "0", 0);
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

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("rtp_frame_sender", "logs/" + std::to_string(pid) + "/rtp_frame_sender.log");
#endif
    }


    /* Destructor() */
    RTPFrameSender::~RTPFrameSender() {
      avcodec_close(encoderContext);
      av_frame_free(&encodingFrame);
    }


    /* Run() */
    raft::kstatus RTPFrameSender::run() {
      AVPacket encodingPacket;
      av_init_packet(&encodingPacket);

      auto &inFrame( input["in_frame"].template peek<mxre::types::Frame>() );
      if(inFrame.rows != (size_t)height || inFrame.cols != (size_t)width) {
        debug_print("inMat size is not compatible.");
        exit(1);
      }
#ifdef __PROFILE__
      double encodingTimeStamp;
      startTimeStamp = getTimeStampNow();
#endif

      cv::cvtColor(inFrame.useAsCVMat(), yuvFrame, cv::COLOR_RGB2YUV_YV12);
      av_image_fill_arrays(encodingFrame->data, encodingFrame->linesize, yuvFrame.data,
                           static_cast<AVPixelFormat>(encodingFrame->format),
                           encodingFrame->width, encodingFrame->height, 1);

      int ret = avcodec_send_frame(encoderContext, encodingFrame);
      while (ret >= 0) {
        ret = avcodec_receive_packet(encoderContext, &encodingPacket);
        if(ret == 0) {
#ifdef __PROFILE__
          encodingTimeStamp = getTimeStampNow();
#endif
          int sentSize = encodingPacket.size;
          if(rtpSender.sendWithTrackingInfo(encodingPacket.data, encodingPacket.size,
                                            inFrame.index, inFrame.timestamp)) {
#ifdef __PROFILE__
            endTimeStamp = getTimeStampNow();
            logger->info("encodingTime/rtpSendingTime/KernelExeTime/Sent Size\t{}\t {}\t {}\t {}",
                encodingTimeStamp - startTimeStamp,
                endTimeStamp - encodingTimeStamp,
                endTimeStamp - startTimeStamp,
                sentSize);
#endif

          }
        }
        av_packet_unref(&encodingPacket);
      }

      inFrame.release();
      recyclePort("in_frame");
      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

