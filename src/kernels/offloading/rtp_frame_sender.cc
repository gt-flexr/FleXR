#include "kernels/offloading/rtp_frame_sender.h"
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {

    /* Constructor() */
    RTPFrameSender::RTPFrameSender(std::string encoder, std::string destAddr,
        int destPort, int bitrate, int fps, int width, int height) : encoder(encoder), bitrate(bitrate),
    fps(fps), width(width), height(height), framePts(0)
    {
      addInputPort<mxre::types::Frame>("in_data");
      this->filename = "rtp://" + destAddr + ":" + std::to_string(destPort);

      av_register_all();
      avcodec_register_all();
      avformat_network_init();

      setRTPContext();
      setRTPStreamWithCodec();
      setFrameWithScaler();
      sendSDP(destAddr, destPort);

      publisher = zmq::socket_t(ctx, zmq::socket_type::pub);
      std::string bindingAddr = "tcp://*:" + std::to_string(destPort+1);
      publisher.bind(bindingAddr);
      publisher.set(zmq::sockopt::conflate, 1);

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("rtp_frame_sender", "logs/" + std::to_string(pid) + "/rtp_frame_sender.log");
#endif
    }


    /* Destructor() */
    RTPFrameSender::~RTPFrameSender() {
      clearSession();
      avformat_network_deinit();

      publisher.close();
      ctx.shutdown();
      ctx.close();
    }


    /* setRTPContext() */
    void RTPFrameSender::setRTPContext() {
      int ret = 0;
      rtpContext = avformat_alloc_context();

      rtpContext->oformat = av_guess_format("rtp", NULL, NULL);
      if(!rtpContext->oformat) {
        clearSession();
        debug_print("av_guess_format");
        exit(1);
      }

      std::strcpy(rtpContext->filename, filename.c_str());
      rtpContext->flags = AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;

      if(!(rtpContext->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open2(&rtpContext->pb, rtpContext->filename, AVIO_FLAG_WRITE, NULL, NULL);
        if(ret < 0) {
          clearSession();
          debug_print("avio_open2");
          exit(1);
        }
      }
    }


    /* setRTPStream() */
    void RTPFrameSender::setRTPStreamWithCodec() {
      int ret = 0;
      AVRational dstFps = {fps, 1};

      // find codec
      rtpCodec = avcodec_find_encoder_by_name(encoder.c_str());
      if(!(rtpCodec)) {
        clearSession();
        debug_print("avcodec_find_encoder_by_name");
        exit(1);
      }

      // create stream
      rtpStream = avformat_new_stream(rtpContext, rtpCodec);
      rtpStream->id = rtpContext->nb_streams - 1;

      // set codec context https://ffmpeg.org/doxygen/4.0/structAVCodecContext.html
      rtpCodecContext = rtpStream->codec;

      rtpCodecContext->codec_id = rtpCodec->id;
      rtpCodecContext->width = width;
      rtpCodecContext->height = height;
      rtpCodecContext->bit_rate = bitrate;
      rtpCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
      rtpCodecContext->time_base = rtpStream->time_base = av_inv_q(dstFps);
      rtpCodecContext->framerate = dstFps;
      rtpCodecContext->delay = 0;        // https://bit.ly/2NlAtEl
      rtpCodecContext->gop_size = 12;     // https://bit.ly/2NtOGit
      rtpCodecContext->max_b_frames = 0; // https://bit.ly/3pIMUal

      debug_print("rtpCodecContext->codec->name: %s", rtpCodecContext->codec->name);

      if (strcmp(rtpCodecContext->codec->name, "libx264") == 0) {
        debug_print("libx264 codec setting...");
        av_opt_set(rtpCodecContext->priv_data, "preset", "ultrafast", 0);
        av_opt_set(rtpCodecContext->priv_data, "tune", "zerolatency", 0);
        av_opt_set(rtpCodecContext->priv_data, "vsink", "0", 0);
      }
      if (strcmp(rtpCodecContext->codec->name, "h264_nvenc") == 0) {
        debug_print("h264_nvenc codec setting...");
        av_opt_set(rtpCodecContext->priv_data, "preset", "ll", 0);
        av_opt_set(rtpCodecContext->priv_data, "zerolatency", "true", 0);
        av_opt_set(rtpCodecContext->priv_data, "delay", 0, 0);
        av_opt_set(rtpCodecContext->priv_data, "2pass", "false", 0);
        av_opt_set(rtpCodecContext->priv_data, "vsink", "0", 0);
      }
      if (strcmp(rtpCodecContext->codec->name, "mjpeg") == 0) {
        debug_print("mjpeg codec setting...");
        rtpCodecContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
        rtpCodecContext->flags = AV_CODEC_FLAG_QSCALE;
        rtpCodecContext->global_quality = FF_QP2LAMBDA * 3.0;
        av_opt_set(rtpCodecContext->priv_data, "huffman", "0", 0);
      }

      ret = avcodec_open2(rtpCodecContext, rtpCodec, NULL);
      if(ret < 0) {
        clearSession();
        debug_print("avcodec_open2");
        exit(1);
      }
    }


    /* setFrameWithScaler() */
    void RTPFrameSender::setFrameWithScaler() {
      uint8_t *rtpFrameBuffer;
      unsigned int rtpFrameBufferSize;

      rtpFrame = av_frame_alloc();
      rtpFrame->width = width;
      rtpFrame->height = height;
      rtpFrame->format = static_cast<int>(rtpCodecContext->pix_fmt);

      rtpFrameBufferSize = avpicture_get_size(rtpCodecContext->pix_fmt, width, height);
      rtpFrameBuffer = new uint8_t[rtpFrameBufferSize];
      avpicture_fill(reinterpret_cast<AVPicture*>(rtpFrame), rtpFrameBuffer, rtpCodecContext->pix_fmt,
          width, height);

      swsContext = sws_getCachedContext(NULL, width, height, AV_PIX_FMT_RGB24,
          width, height, rtpCodecContext->pix_fmt,
          SWS_BICUBIC, NULL, NULL, NULL);
    }


    /* sendSDP() */
    void RTPFrameSender::sendSDP(std::string &destAddr, int port) {
      zmq::context_t sdpCtx;
      zmq::socket_t sdpSock;
      char buf[SDP_BUF_SIZE];

      // 1. Generate SDP
      int ret = avformat_write_header(rtpContext, NULL);
      if(ret < 0) {
        clearSession();
        debug_print("avformat_write_header");
        exit(1);
      }

      AVFormatContext *ac[] = { rtpContext };
      av_sdp_create(ac, 1, buf, SDP_BUF_SIZE);
      debug_print("===== SDP =====\n%s\n", buf);

      // 2. Send the generated SDP to the receiver
      sdpSock = zmq::socket_t(sdpCtx, zmq::socket_type::req);
      std::string connectingAddr = "tcp://" + destAddr + ":" + std::to_string(port);
      sdpSock.connect(connectingAddr);

      zmq::message_t sendingSDP(buf, SDP_BUF_SIZE), ackMsg;
      sdpSock.send(sendingSDP, zmq::send_flags::none);
      auto res = sdpSock.recv(ackMsg);

      if( strcmp((const char*)ackMsg.data(), "ACK") == 0 ) {
        debug_print("SDP is successfully delivered");
      }
      else {
        debug_print("%s", (const char*)ackMsg.data());
      }

      // 3. Clear the connection
      sdpSock.close();
      sdpCtx.shutdown();
      sdpCtx.close();
    }


    /* clearSession() */
    void RTPFrameSender::clearSession() {
      if(rtpCodecContext) avcodec_close(rtpCodecContext);
      if(rtpFrame){
        delete[] rtpFrame->data[0];
        av_frame_free(&rtpFrame);
      }
      if(!(rtpContext->oformat->flags & AVFMT_NOFILE)) avio_close(rtpContext->pb);
      if(rtpContext) avformat_free_context(rtpContext);
    }


    /* Run() */
    raft::kstatus RTPFrameSender::run() {
      auto &inData( input["in_data"].template peek<mxre::types::Frame>() );
      if(inData.rows != (size_t)height || inData.cols != (size_t)width) {
        clearSession();
        debug_print("inMat size is not compatible.");
        exit(1);
      }

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      int ret=-1, gotPkt=0;

      // convert cvframe into ffmpeg frame
      const int stride[] = {static_cast<int>(inData.useAsCVMat().step[0])};
      sws_scale(swsContext, &inData.data, stride, 0, inData.rows, rtpFrame->data, rtpFrame->linesize);
      rtpFrame->pts = framePts++;

      // encode video frame
      AVPacket packet;
      packet.data = nullptr;
      packet.size = 0;
      av_init_packet(&packet);

      ret = avcodec_encode_video2(rtpCodecContext, &packet, rtpFrame, &gotPkt);

      // send the encoded frame as packet
      if(gotPkt == 1 && ret == 0) {
        packet.pts = av_rescale_q_rnd(packet.pts, rtpCodecContext->time_base, rtpStream->time_base,
                                      AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.dts = av_rescale_q_rnd(packet.dts, rtpCodecContext->time_base, rtpStream->time_base,
                                      AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        packet.duration = av_rescale_q(packet.duration, rtpCodecContext->time_base, rtpStream->time_base);
        packet.stream_index = rtpStream->index;

        /* Write the compressed frame to the media file. */
        int rtpWritingResult = av_interleaved_write_frame(rtpContext, &packet);

        if(rtpWritingResult == 0) {
          // Send Frame Tracking Info
          mxre::types::FrameTrackingInfo frameTrackingInfo;
          frameTrackingInfo.index = inData.index;
          frameTrackingInfo.timestamp = inData.timestamp;
          debug_print("%d, %lf", frameTrackingInfo.index, frameTrackingInfo.timestamp);
          publisher.send(zmq::buffer(&frameTrackingInfo, sizeof(frameTrackingInfo)), zmq::send_flags::none);

#ifdef __PROFILE__
          endTimeStamp = getTimeStampNow();
          logger->info("\t{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif
        }
      }

      inData.release();
      input["in_data"].recycle(1);
      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

