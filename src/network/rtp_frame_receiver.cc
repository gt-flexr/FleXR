#include <include/network/rtp_frame_receiver.h>

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      /* Constructor() */
      RTPFrameReceiver::RTPFrameReceiver(std::string decoder, std::string sdp, int width, int height) :
        decoder(decoder), sdp(sdp), width(width), height(height), raft::kernel()
      {
        output.addPort<cv::Mat>("out_data");

        av_register_all();
        avcodec_register_all();
        avformat_network_init();
        protocolWhitelist = NULL;

        initRTPContext();
        initRTPCodecAndScaler();
        initFrame();
      }


      /* Destructor() */
      RTPFrameReceiver::~RTPFrameReceiver() {
        clearSession();
        avformat_network_deinit();
      }


      /* setRTPContext() */
      void RTPFrameReceiver::initRTPContext() {
        int ret = 0;
        rtpContext = avformat_alloc_context();
        rtpContext->flags = AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;

        // set protocol whitelist to receive
        av_dict_set(&protocolWhitelist, "protocol_whitelist", "file,udp,rtp,crypto", 0);
        ret = avformat_open_input(&rtpContext, sdp.c_str(), NULL, &protocolWhitelist);
        if(ret < 0) {
          clearSession();
          debug_print("avformat_open_input");
          exit(1);
        }

        // set stream info into rtpContext
        ret = avformat_find_stream_info(rtpContext, NULL);
        if(ret < 0) {
          clearSession();
          debug_print("avformat_find_stream_info");
          exit(1);
        }

        // find video stream
        for(unsigned int i = 0; i < rtpContext->nb_streams; i++) {
          if(rtpContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            rtpStreamIndex = i;
            debug_print("rtpStreamIndex: %d", rtpStreamIndex);
          }
        }

        av_read_play(rtpContext);
      }


      /* setRTPCodec() */
      void RTPFrameReceiver::initRTPCodecAndScaler() {
        int ret = 0;

        // find codec
        rtpCodec = avcodec_find_decoder_by_name(decoder.c_str());
        if(!(rtpCodec)) {
          clearSession();
          debug_print("avcodec_find_encoder_by_name");
          exit(1);
        }

        rtpCodecContext = avcodec_alloc_context3(rtpCodec);
        avcodec_get_context_defaults3(rtpCodecContext, rtpCodec);
        avcodec_copy_context(rtpCodecContext, rtpContext->streams[rtpStreamIndex]->codec);

        // sanity check
        if(rtpCodecContext->height != height || rtpCodecContext->width != width) {
          clearSession();
          debug_print("the size of received rtpFrame is not compatible.");
          exit(1);
        }

        ret = avcodec_open2(rtpCodecContext, rtpCodec, NULL);
        if(ret < 0) {
          clearSession();
          debug_print("avcodec_open2");
          exit(1);
        }

        // set scaler
        swsContext = sws_getContext(rtpCodecContext->width, rtpCodecContext->height, rtpCodecContext->pix_fmt,
                                    width, height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
      }


      /* setFrameWithScaler() */
      void RTPFrameReceiver::initFrame() {
        // set received rtpFrame
        rtpFrame = av_frame_alloc();
        rtpFrameSize = avpicture_get_size(AV_PIX_FMT_YUV420P, rtpCodecContext->width, rtpCodecContext->height);
        rtpFrameBuf = std::vector<uint8_t>(rtpFrameSize);
        avpicture_fill(reinterpret_cast<AVPicture*>(rtpFrame), rtpFrameBuf.data(), AV_PIX_FMT_YUV420P,
            rtpCodecContext->width, rtpCodecContext->height);

        convertingFrame = av_frame_alloc();
        convertingFrameSize = avpicture_get_size(AV_PIX_FMT_BGR24, width, height);
        convertingFrameBuf = std::vector<uint8_t>(convertingFrameSize);
        avpicture_fill(reinterpret_cast<AVPicture*>(convertingFrame), convertingFrameBuf.data(), AV_PIX_FMT_BGR24,
            width, height);
      }


      /* clearSession() */
      void RTPFrameReceiver::clearSession() {
        if(rtpCodecContext) avcodec_close(rtpCodecContext);

        if(rtpFrame){
          delete [] rtpFrame->data[0];
          av_frame_free(&rtpFrame);
        }

        if(convertingFrame) {
          delete [] convertingFrame->data[0];
          av_frame_free(&convertingFrame);
        }

        if(!(rtpContext->oformat->flags & AVFMT_NOFILE)) avio_close(rtpContext->pb);
        if(rtpContext) avformat_free_context(rtpContext);
      }


      /* Run() */
      raft::kstatus RTPFrameReceiver::run() {
        auto &outData( output["out_data"].allocate<cv::Mat>() );
        int ret = 0, receivedFrame = 0, readSuccess = -1;

        AVPacket packet;
        av_init_packet(&packet);

        readSuccess = av_read_frame(rtpContext, &packet);

        if(packet.stream_index == rtpStreamIndex && readSuccess >= 0) {
          int result = avcodec_decode_video2(rtpCodecContext, rtpFrame, &receivedFrame, &packet);
          if(receivedFrame) {
            sws_scale(swsContext, rtpFrame->data, rtpFrame->linesize, 0, rtpFrame->height,
                convertingFrame->data, convertingFrame->linesize);

            outData = cv::Mat(rtpCodecContext->height, rtpCodecContext->width, CV_8UC3, convertingFrameBuf.data(),
                              convertingFrame->linesize[0]).clone();
            output["out_data"].send();
          }
        }
        av_free_packet(&packet);
        return raft::proceed;
      }
    } // namespace network
  } // namespace pipeline
} // namespace mxre

