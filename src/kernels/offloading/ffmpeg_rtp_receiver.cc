#include "kernels/offloading/ffmpeg_rtp_receiver.h"
#include <libavcodec/avcodec.h>
#include <opencv2/imgproc.hpp>
#include <unistd.h>
extern"C" {
  #include <libavutil/dict.h>
}

namespace mxre
{
  namespace kernels
  {
    /* Constructor() */
    FFmpegRTPReceiver::FFmpegRTPReceiver(std::string decoder, std::string srcAddr, int destPort, int width, int height) :
      MXREKernel(), decoder(decoder),width(width), height(height)
    {
      addOutputPort<mxre::types::Frame>("out_data");

      av_register_all();
      avcodec_register_all();
      avformat_network_init();
      av_log_set_level(AV_LOG_TRACE);
      rtpContextOptions = NULL;

      // recv sdp and save it as a file
      sdp = "/tmp/" + std::to_string(pid) + "_" + std::to_string(destPort) + ".sdp";
      recvSDP(destPort);

      initRTPContext();
      initRTPCodecAndScaler();
      initFrame();

      subscriber = zmq::socket_t(ctx, zmq::socket_type::sub);
      std::string connectingAddr = "tcp://" + srcAddr + ":" + std::to_string(destPort+1);
      subscriber.connect(connectingAddr);
      subscriber.set(zmq::sockopt::conflate, 1);
      subscriber.set(zmq::sockopt::subscribe, "");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("rtp_frame_receiver", "logs/" + std::to_string(pid) + "/rtp_frame_receiver.log");
#endif
    }


    /* Destructor() */
    FFmpegRTPReceiver::~FFmpegRTPReceiver() {
      clearSession();
      avformat_network_deinit();

      subscriber.close();
      ctx.shutdown();
      ctx.close();
    }


    /* recvSDP() */
    void FFmpegRTPReceiver::recvSDP(int destPort) {
      zmq::context_t sdpCtx;
      zmq::socket_t sdpSock;

      char buf[SDP_BUF_SIZE];
      zmq::message_t ackMsg("ACK\0", 4);
      sdpSock = zmq::socket_t(sdpCtx, zmq::socket_type::rep);
      std::string bindingAddr = "tcp://*:" + std::to_string(destPort);
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
    void FFmpegRTPReceiver::initRTPContext() {
      int ret = 0;
      rtpContext = avformat_alloc_context();

      // set protocol whitelist to receive
      av_dict_set(&rtpContextOptions, "protocol_whitelist", "file,udp,rtp,crypto", 0);
      ret = avformat_open_input(&rtpContext, sdp.c_str(), NULL, &rtpContextOptions);
      if(ret < 0) {
        clearSession();
        debug_print("avformat_open_input");
        exit(1);
      }

      //rtpContext->flags = AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;

      // set stream info into rtpContext
      AVDictionary *dict;
      AVDictionaryEntry *dictElem = NULL;
      ret = avformat_find_stream_info(rtpContext, &dict);
      if(ret < 0) {
        clearSession();
        debug_print("avformat_find_stream_info");
        exit(1);
      }
      while (dictElem = av_dict_get(dict, "", dictElem, AV_DICT_IGNORE_SUFFIX)) {
        printf("key:%10s  |  value:%s\n", dictElem->key, dictElem->value);
      }

      rtpContext->max_delay = 0;
      rtpContext->max_ts_probe = 0;
      rtpContext->flags = AVFMT_FLAG_NONBLOCK|AVFMT_FLAG_IGNDTS|AVFMT_FLAG_NOFILLIN|AVFMT_FLAG_NOPARSE|AVFMT_FLAG_FLUSH_PACKETS|AVFMT_FLAG_DISCARD_CORRUPT|AVFMT_FLAG_NOBUFFER;
      printf("RTP CONTEXT===== \n");
      printf("bitrate: %ld \n", rtpContext->bit_rate);
      printf("maxDelay: %d \n", rtpContext->max_delay);
      printf("max_analyze_duration: %ld \n", rtpContext->max_analyze_duration);
      printf("max_chunk_duration: %d \n", rtpContext->max_chunk_duration);
      printf("max_chunk_size: %d \n", rtpContext->max_chunk_size);
      printf("max_index_size: %d \n", rtpContext->max_index_size);
      printf("max_interleave_delta: %ld \n", rtpContext->max_interleave_delta);
      printf("max_picture_buffer: %d \n", rtpContext->max_picture_buffer);
      printf("max_streams: %d \n", rtpContext->max_streams);
      printf("max_ts_probe: %d \n", rtpContext->max_ts_probe);
      printf("packet_size: %d \n", rtpContext->packet_size);
      printf("probesize: %ld \n", rtpContext->probesize);

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
    void FFmpegRTPReceiver::initRTPCodecAndScaler() {
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
      printf("RTP CODEC CONTEXT===== \n");
      printf("bitrate: %ld \n", rtpCodecContext->bit_rate);
      printf("time_base: %d/%d\n", rtpCodecContext->time_base.num, rtpCodecContext->time_base.den);
      printf("framerate: %d/%d\n", rtpCodecContext->framerate.num, rtpCodecContext->framerate.den);
      printf("has/max b frame: %d/%d\n", rtpCodecContext->has_b_frames, rtpCodecContext->max_b_frames);
      printf("coded width/height: %d/%d\n", rtpCodecContext->coded_width, rtpCodecContext->coded_height);
      printf("delay: %d\n", rtpCodecContext->delay);
      printf("frame_size/bits: %d/%d\n", rtpCodecContext->frame_size, rtpCodecContext->frame_bits);
      printf("gop_size: %d \n", rtpCodecContext->gop_size);
      printf("width/height: %d/%d \n\n", rtpCodecContext->width, rtpCodecContext->height);

      rtpCodecContext->delay = 0;
      //rtpCodecContext->gop_size = 12;
      rtpCodecContext->max_b_frames = 0;

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
    }


    /* setFrameWithScaler() */
    void FFmpegRTPReceiver::initFrame() {
      // set received rtpFrame
      rtpFrame = av_frame_alloc();
      rtpFrame->width = width;
      rtpFrame->height = height;
      rtpFrame->format = AV_PIX_FMT_YUV420P;

      rtpFrameSize = avpicture_get_size(static_cast<AVPixelFormat>(rtpFrame->format),
                                        rtpFrame->width, rtpFrame->height);
      rtpFrameBuf = (uint8_t*)av_malloc(rtpFrameSize);

      avpicture_fill(reinterpret_cast<AVPicture*>(rtpFrame), rtpFrameBuf,
                     static_cast<AVPixelFormat>(rtpFrame->format), rtpFrame->width, rtpFrame->height);

      convertingFrame = av_frame_alloc();
      convertingFrameSize = avpicture_get_size(AV_PIX_FMT_BGR24, rtpFrame->width, rtpFrame->height);
      convertingFrameBuf = (uint8_t*)av_malloc(convertingFrameSize);
      avpicture_fill(reinterpret_cast<AVPicture*>(convertingFrame), convertingFrameBuf,
                     AV_PIX_FMT_BGR24, width, height);
    }


    /* clearSession() */
    void FFmpegRTPReceiver::clearSession() {
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
    raft::kstatus FFmpegRTPReceiver::run() {

      auto &outData( output["out_data"].allocate<mxre::types::Frame>() );
      outData = mxre::types::Frame(height, width, CV_8UC3, -1, -1);
      cv::Mat yuvFrame = cv::Mat::zeros(height*1.5, width, CV_8UC1);
      int receivedFrame = 0, readSuccess = -1;

      AVPacket packet;
      av_init_packet(&packet);

      mxre::types::FrameTrackingInfo frameTrackingInfo;

      // 1. Read the packet via RTP context
      readSuccess = av_read_frame(rtpContext, &packet);

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      // 2. Decode the received packet
      if(packet.stream_index == rtpStreamIndex && readSuccess == 0) { // check the sucess of RTP recv
        // 2.1. Receive the frame tracking info
        subscriber.recv( zmq::buffer(&frameTrackingInfo, sizeof(frameTrackingInfo)) );

        // 2.2. Decode the received packet
        int result = avcodec_decode_video2(rtpCodecContext, rtpFrame, &receivedFrame, &packet);

        if(receivedFrame != 0) { // check the decoded frame
          outData.index = frameTrackingInfo.index;
          outData.timestamp = frameTrackingInfo.timestamp;

          // 2.3. Convert received YUV frame into RGB24
          AVPicture* rtpFrameAsPicture = reinterpret_cast<AVPicture*>(rtpFrame);
          avpicture_layout(rtpFrameAsPicture, static_cast<AVPixelFormat>(rtpFrame->format),
                           rtpFrame->width, rtpFrame->height, yuvFrame.data, yuvFrame.total());

          if(decoder == "h264") cv::cvtColor(yuvFrame, outData.useAsCVMat(), cv::COLOR_YUV420p2RGB);
          else if(decoder == "h264_cuvid") cv::cvtColor(yuvFrame, outData.useAsCVMat(), cv::COLOR_YUV2BGR_NV12);

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

