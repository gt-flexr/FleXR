#include "kernels/offloading/rtp_frame_receiver.h"
#include <libavcodec/avcodec.h>
#include <opencv2/imgproc.hpp>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor() */
    RTPFrameReceiver::RTPFrameReceiver(int portBase, std::string decoderName, int width, int height):
      MXREKernel(), decoderName(decoderName), width(width), height(height)
    {
      addOutputPort<mxre::types::Frame>("out_frame");

      // RTP Streaming
      rtpSession = rtpContext.create_session("127.0.0.1");
      rtpStream = rtpSession->create_stream(49991, -1, RTP_FORMAT_GENERIC, RCE_FRAGMENT_GENERIC);

      // Frame Tracking
      subscriber = zmq::socket_t(ctx, zmq::socket_type::sub);
      std::string connectingAddr = "tcp://localhost:" + std::to_string(portBase+1);
      subscriber.connect(connectingAddr);
      subscriber.set(zmq::sockopt::conflate, 1);
      subscriber.set(zmq::sockopt::subscribe, "");

      // Decoder
      av_register_all();
      avcodec_register_all();

      decoder = avcodec_find_decoder_by_name(decoderName.c_str());
      if(!(decoder)) {
        debug_print("decoder %s is not found", decoderName.c_str());
        exit(1);
      }

      decoderContext = avcodec_alloc_context3(decoder);
      //decoderContext->delay = 0;
      decoderContext->max_b_frames = 0;

      int ret = avcodec_open2(decoderContext, decoder, NULL);
      if(ret < 0) {
        debug_print("decoder %s open failed.", decoderName.c_str());
        exit(1);
      }

      decodingFrame = av_frame_alloc();
      decodingFrame->width = width; decodingFrame->height = height;
      decodingFrame->format = AV_PIX_FMT_YUV420P;
      decodingFrameSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, width, height, 1);;
      decodingFrameBuffer = (uint8_t*)av_malloc(decodingFrameSize);
      av_image_fill_arrays(decodingFrame->data, decodingFrame->linesize, decodingFrameBuffer, AV_PIX_FMT_YUV420P,
                           width, height, 1);
      yuvFrame = cv::Mat::zeros(height*1.5, width, CV_8UC1);

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("rtp_frame_receiver", "logs/" + std::to_string(pid) + "/rtp_frame_receiver.log");
#endif
    }


    /* Destructor() */
    RTPFrameReceiver::~RTPFrameReceiver() {
      rtpContext.destroy_session(rtpSession);
      subscriber.close();
      ctx.shutdown();
      ctx.close();

      av_frame_free(&decodingFrame);
      avcodec_close(decoderContext);
    }

    /* Run() */
    raft::kstatus RTPFrameReceiver::run() {
      auto &outFrame( output["out_frame"].allocate<mxre::types::Frame>() );
      outFrame = mxre::types::Frame(height, width, CV_8UC3, -1, -1);
      rtpFrame = nullptr;
      AVPacket decodingPacket;
      mxre::types::FrameTrackingInfo trackingInfo;
      int ret = 0;


      rtpFrame = rtpStream->pull_frame();
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      if(rtpFrame != nullptr) {
        subscriber.recv(zmq::buffer(&trackingInfo, sizeof(mxre::types::FrameTrackingInfo)) );
        outFrame.index = trackingInfo.index;
        outFrame.timestamp = trackingInfo.timestamp;

        av_packet_from_data(&decodingPacket, rtpFrame->payload, rtpFrame->payload_len);
        ret = avcodec_send_packet(decoderContext, &decodingPacket);
        while (ret >= 0) {
          ret = avcodec_receive_frame(decoderContext, decodingFrame);
          if(ret == 0) {
            av_image_copy_to_buffer(yuvFrame.data, yuvFrame.total(), decodingFrame->data, decodingFrame->linesize,
                                    static_cast<AVPixelFormat>(decodingFrame->format),
                                    decodingFrame->width, decodingFrame->height, 1);
            if(decoderName == "h264") cv::cvtColor(yuvFrame, outFrame.useAsCVMat(), cv::COLOR_YUV420p2RGB);
            else if(decoderName == "h264_cuvid") cv::cvtColor(yuvFrame, outFrame.useAsCVMat(), cv::COLOR_YUV2BGR_NV12);

            output["out_frame"].send();
            sendFrameCopy("out_frame", &outFrame);

#ifdef __PROFILE__
            endTimeStamp = getTimeStampNow();
            logger->info("RecvTime/ExportTime/ExeTime\t{}\t {}\t {}", startTimeStamp, endTimeStamp,
                endTimeStamp-startTimeStamp);
#endif
            (void)uvg_rtp::frame::dealloc_frame(rtpFrame);
          }
        }
      }

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

