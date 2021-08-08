#include <kernels/source/rtp_frame_receiver.h>
#include <utils/msg_sending_functions.h>
#include <libavcodec/avcodec.h>
#include <opencv2/imgproc.hpp>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {
    /* Constructor() */
    RTPFrameReceiver::RTPFrameReceiver(int port, std::string decoderName, int width, int height):
      rtpReceiver("127.0.0.1", port),
      width(width), height(height), decoderName(decoderName),
      FleXRKernel()
    {
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, 0, 0);

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
    }


    /* Destructor() */
    RTPFrameReceiver::~RTPFrameReceiver()
    {
      av_frame_free(&decodingFrame);
      avcodec_close(decoderContext);
    }

    /* Run() */
    raft::kstatus RTPFrameReceiver::run()
    {
      FrameReceiverMsgType *outFrame = portManager.getOutputPlaceholder<FrameReceiverMsgType>("out_frame");

      outFrame->data = types::Frame(height, width, CV_8UC3);
      AVPacket decodingPacket;
      int ret = 0;

      uint8_t *recvDataBuffer = nullptr;
      uint32_t recvDataSize = 0;

      if(rtpReceiver.receiveDynamicWithTrackingInfo(&recvDataBuffer, recvDataSize,
                                                    outFrame->tag, outFrame->seq, outFrame->ts))
      {
        double st = getTsNow();
        av_packet_from_data(&decodingPacket, recvDataBuffer, recvDataSize);
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

            double et = getTsNow();
            debug_print("decodeTime(%lf), recvInfo %s:%d size(%d)", et-st, outFrame->tag, outFrame->seq, recvDataSize);
            if(logger.isSet()) logger.getInstance()->info("st/et/decodingTime/recvSize\t{}\t {}\t {}\t {}",
                                                          st, et, et-st, recvDataSize);

            portManager.sendOutput("out_frame", outFrame);
          }
        }

        delete recvDataBuffer;
      }

      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

