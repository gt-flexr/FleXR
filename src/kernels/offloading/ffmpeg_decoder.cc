#include "kernels/offloading/ffmpeg_decoder.h"
#include <opencv2/imgproc.hpp>
#include <unistd.h>


namespace mxre
{
  namespace kernels
  {
    /* Constructor() */
    FFmpegDecoder::FFmpegDecoder(std::string decoderName, int width, int height) :
      MXREKernel(), decoderName(decoderName),width(width), height(height)
    {
      addInputPort<mxre::types::AVEncodedFrame>("in_encoded_frame");
      addOutputPort<mxre::types::Frame>("out_frame");

      av_register_all();
      avcodec_register_all();

      decoder = avcodec_find_decoder_by_name(decoderName.c_str());
      if(!(decoder)) {
        debug_print("decoder %s is not found", decoderName.c_str());
        exit(1);
      }

      decoderContext = avcodec_alloc_context3(decoder);
      decoderContext->delay = 0;
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
      av_image_fill_arrays(decodingFrame->data, decodingFrame->linesize, decodingFrameBuffer, AV_PIX_FMT_YUV420P, width, height, 1);

      yuvFrame = cv::Mat::zeros(height*1.5, width, CV_8UC1);

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("ffmpeg_decoder", "logs/" + std::to_string(pid) + "/ffmpeg_decoder.log");
#endif
    }


    /* Destructor() */
    FFmpegDecoder::~FFmpegDecoder() {
      av_frame_free(&decodingFrame);
      avcodec_close(decoderContext);
    }


    /* Run() */
    raft::kstatus FFmpegDecoder::run() {
      auto &inEncodedFrame( input["in_encoded_frame"].template peek<mxre::types::AVEncodedFrame>() );
      auto &outFrame( output["out_frame"].allocate<mxre::types::Frame>() );
      outFrame = mxre::types::Frame(height, width, CV_8UC3, -1, -1);

      printf("inEncodedFrame.trackingInfo.index %d\n", inEncodedFrame.trackingInfo.index);
      printf("inEncodedFrame.trackingInfo.timestamp %f\n", inEncodedFrame.trackingInfo.timestamp);
      printf("inEncodedFrame.encodedFrame %p\n", inEncodedFrame.encodedFrame);
      printf("inEncodedFrame.encodedFrame.size %d(%p)\n\n", inEncodedFrame.encodedFrame->size, inEncodedFrame.encodedFrame->data);

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      debug_print("flag1...");
      int ret = avcodec_send_packet(decoderContext, inEncodedFrame.encodedFrame);
      while (ret >= 0) {
        ret = avcodec_receive_frame(decoderContext, decodingFrame);
        debug_print("flag2...");
        if(ret == 0) {
          debug_print("ret == 0 DECODE...1");
          av_image_copy_to_buffer(yuvFrame.data, yuvFrame.total(), decodingFrame->data, decodingFrame->linesize,
                                  static_cast<AVPixelFormat>(decodingFrame->format),
                                  decodingFrame->width, decodingFrame->height, 1);

          if(decoderName == "h264") cv::cvtColor(yuvFrame, outFrame.useAsCVMat(), cv::COLOR_YUV420p2RGB);
          else if(decoderName == "h264_cuvid") cv::cvtColor(yuvFrame, outFrame.useAsCVMat(), cv::COLOR_YUV2BGR_NV12);

          debug_print("ret == 0 DECODE...2");
          output["out_frame"].send();
          sendFrameCopy("out_frame", &outFrame);

          debug_print("ret == 0 DECODE...3");
#ifdef __PROFILE__
          endTimeStamp = getTimeStampNow();
          logger->info("RecvTime/ExportTime/ExeTime\t{}\t {}\t {}", startTimeStamp, endTimeStamp,
              endTimeStamp-startTimeStamp);
#endif
        }
      }

      if(inEncodedFrame.encodedFrame->data){
        debug_print("delete");
        delete[] inEncodedFrame.encodedFrame->data;
        debug_print("delete Done");
      }
      delete [] inEncodedFrame.encodedFrame;

      debug_print("EXIT");
      //av_packet_unref(&inEncodedFrame.encodedFrame);
      recyclePort("in_encoded_frame");

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

