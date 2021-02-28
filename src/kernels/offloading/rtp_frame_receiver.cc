#include <kernels/offloading/rtp_frame_receiver.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor() */
    RTPFrameReceiver::RTPFrameReceiver(std::string decoder, int srcPort, int width, int height) :
      MXREKernel(), decoder(decoder),width(width), height(height)
    {
      addOutputPort<mxre::types::Frame>("out_data");

      av_register_all();
      avcodec_register_all();
      avformat_network_init();
      protocolWhitelist = NULL;

      // recv sdp and save it as a file
      sdp = "/tmp/" + std::to_string(pid) + "_" + std::to_string(srcPort) + ".sdp";
      recvSDP(srcPort);

      initRTPContext();
      initRTPCodecAndScaler();
      initFrame();

      subscriber = zmq::socket_t(ctx, zmq::socket_type::sub);
      std::string connectingAddr = "tcp://*:" + std::to_string(srcPort+1);
      subscriber.connect(connectingAddr);
      subscriber.set(zmq::sockopt::conflate, 1);

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("rtp_frame_receiver", "logs/" + std::to_string(pid) + "/rtp_frame_receiver.log");
#endif
    }


    /* Destructor() */
    RTPFrameReceiver::~RTPFrameReceiver() {
      clearSession();
      avformat_network_deinit();

      subscriber.close();
      ctx.shutdown();
      ctx.close();
    }


    /* recvSDP() */
    void RTPFrameReceiver::recvSDP(int srcPort) {
      zmq::context_t sdpCtx;
      zmq::socket_t sdpSock;

      char buf[SDP_BUF_SIZE];
      zmq::message_t ackMsg("ACK", 3);
      sdpSock = zmq::socket_t(sdpCtx, zmq::socket_type::rep);
      std::string bindingAddr = "tcp://*:" + std::to_string(srcPort);
      sdpSock.bind(bindingAddr);

      // 2. Recv the sdp
      sdpSock.recv( zmq::buffer(buf, SDP_BUF_SIZE) );

      // 3. Store it as a file
      FILE* fsdp = fopen(sdp.c_str(), "w");
      fprintf(fsdp, "%s", buf);
      fclose(fsdp);

      // 4. Send an ack
      sdpSock.send(ackMsg, zmq::send_flags::none);

      sdpSock.close();
      sdpCtx.shutdown();
      sdpCtx.close();
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
      rtpCodecContext->delay = 0;
      rtpCodecContext->gop_size = 12;
      rtpCodecContext->max_b_frames = 0;
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

      auto &outData( output["out_data"].allocate<mxre::types::Frame>() );
      int ret = 0, receivedFrame = 0, readSuccess = -1;

      AVPacket packet;
      av_init_packet(&packet);

      mxre::types::FrameTrackingInfo frameTrackingInfo;

      // Recv Frame Tracking Info and Frame
      subscriber.recv( zmq::buffer(&frameTrackingInfo, sizeof(frameTrackingInfo)) );
      debug_print("%d, %lf", frameTrackingInfo.index, frameTrackingInfo.timestamp);
      readSuccess = av_read_frame(rtpContext, &packet);

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      /* Received Packet Status */

      if(packet.stream_index == rtpStreamIndex && readSuccess >= 0) {
        int result = avcodec_decode_video2(rtpCodecContext, rtpFrame, &receivedFrame, &packet);

        if(receivedFrame) {
          sws_scale(swsContext, rtpFrame->data, rtpFrame->linesize, 0, rtpFrame->height,
              convertingFrame->data, convertingFrame->linesize);

          cv::Mat temp = cv::Mat(rtpCodecContext->height, rtpCodecContext->width, CV_8UC3,
                                 convertingFrameBuf.data(), convertingFrame->linesize[0]);
          outData = mxre::types::Frame(temp, frameTrackingInfo.index, frameTrackingInfo.timestamp);
          debug_print("Frame: %d, %lf", outData.index, outData.timestamp);

          output["out_data"].send();
          sendFrameCopy("out_data", &outData);

#ifdef __PROFILE__
          endTimeStamp = getTimeStampNow();
          logger->info("RecvTime/ExportTime/ExeTime\t{}\t {}\t {}", startTimeStamp, endTimeStamp,
                       endTimeStamp-startTimeStamp);
#endif
        }
      }
      //av_free_packet(&packet);

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

