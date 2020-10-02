#include <iostream>
#include <vector>
// FFmpeg
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
// OpenCV
#include <opencv2/opencv.hpp>

#define WIDTH 640
#define HEIGHT 480
#define FPS 20
#define BITRATE 1000000

using namespace std;
using namespace cv;

void setCamera(VideoCapture *cam, int camIdx) {
  cam->open(camIdx);
  cam->set(CAP_PROP_FRAME_WIDTH, WIDTH);
  cam->set(CAP_PROP_FRAME_HEIGHT, HEIGHT);
}

void encode(AVFormatContext *formatCtx, AVCodecContext *codecCtx, AVFrame *avFrame, AVPacket *packet) {
  int ret;
  ret = avcodec_send_frame(codecCtx, avFrame);
  while (ret >= 0) {
    ret = avcodec_receive_packet(codecCtx, packet);
    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else {
      cerr << "Error while sending a frame for encoding" << endl;
      exit(1);
    }

    av_interleaved_write_frame(formatCtx, packet);
    av_packet_unref(packet);
  }
}

void logPacket(const AVFormatContext *context, const AVPacket *packet)
{
  AVRational *timeBase = &context->streams[packet->stream_index]->time_base;

  /*printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
    av_ts_make_string(pkt->pts), av_ts_make_string(pkt->pts, time_base),
    av_ts_make_string(pkt->dts), av_ts_make_string(pkt->dts, time_base),
    av_ts_make_string(pkt->duration), av_ts_make_string(pkt->duration, time_base),
    pkt->stream_index);*/
}

int writeFrame(AVFormatContext *context, const AVRational *timeBase, AVStream *stream, AVPacket *packet) {
  packet->pts = av_rescale_q_rnd(packet->pts, *timeBase, stream->time_base,
                                 AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
  packet->dts = av_rescale_q_rnd(packet->dts, *timeBase, stream->time_base,
                                 AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
  packet->duration = av_rescale_q(packet->duration, *timeBase, stream->time_base);

  logPacket(context, packet);
  return av_interleaved_write_frame(context, packet);
}


AVStream* addStream(AVFormatContext *context, AVCodec **codec, const char* codecName) {
  AVCodecContext *codecContext;
  AVStream *stream;
  AVRational dstFps = {FPS, 1};

  *codec = avcodec_find_encoder_by_name(codecName);
  if(!(*codec)) {
    cerr << "avcodec_find_decoder_by_name" << endl;
    exit(1);
  }
  stream = avformat_new_stream(context, *codec);

  stream->id = context->nb_streams-1;
  codecContext = stream->codec;

  codecContext->codec_id = (*codec)->id;
  codecContext->width = WIDTH;
  codecContext->height = HEIGHT;
  codecContext->bit_rate = BITRATE;
  codecContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
  //codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
  codecContext->time_base = stream->time_base = av_inv_q(dstFps);
  codecContext->framerate = dstFps;
  codecContext->gop_size = 3;
  codecContext->max_b_frames = 0;

  cout << "codecContext->codec->name = " << codecContext->codec->name << endl;
  if (strcmp(codecContext->codec->name, "libx264") == 0) {
    cout << "libx264!" << endl;
    av_opt_set(codecContext->priv_data, "preset", "slow", 0);
    av_opt_set(codecContext->priv_data, "tune", "zerolatency", 0);
    av_opt_set(codecContext->priv_data, "vsink", "0", 0);
  }
  if (strcmp(codecContext->codec->name, "h264_nvenc") == 0) {
    // ffmpeg -h encoder=h264_nvenc
    cout << "h264_nvenc!" << endl;
    av_opt_set(codecContext->priv_data, "preset", "fast", 0);
    av_opt_set(codecContext->priv_data, "zerolatency", "true", 0);
    av_opt_set(codecContext->priv_data, "delay", 0, 0);
    av_opt_set(codecContext->priv_data, "2pass", "false", 0);
    av_opt_set(codecContext->priv_data, "vsink", "0", 0);
  }
  if (strcmp(codecContext->codec->name, "mjpeg") == 0) {
    cout << "mjpeg!" << endl;
    codecContext->flags = AV_CODEC_FLAG_QSCALE;
    codecContext->global_quality = FF_QP2LAMBDA * 3.0;
    av_opt_set(codecContext->priv_data, "huffman", "0", 0);
  }

  return stream;
}

// 1. OpenCV Camera Frame
// 2. Encoder
// 3. RTP write

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage: cv2ff <codec name>" << std::endl;
    return 1;
  }
  const char* codecName = argv[1];

  /* OpenCV Camera Settings */
  cv::VideoCapture cam;
  setCamera(&cam, 0);
  vector<uint8_t> cvFrameBuf(HEIGHT * WIDTH * 3 + 16);
  Mat cvFrame(HEIGHT, WIDTH, CV_8UC3, cvFrameBuf.data(), WIDTH * 3);


  /* FFmpeg RTP Settings */
  const char *filename = "rtp://0.0.0.0:49983";
  AVFormatContext *rtpContext;
  AVStream *rtpStream;
  AVCodec *rtpCodec;
  AVFrame *rtpFrame;
  double etime;
  int ret;

  av_register_all();
  avcodec_register_all();
  avformat_network_init();

  /*   1. set RTP context */ cout << "==1==" << endl;
  rtpContext = avformat_alloc_context();
  rtpContext->oformat = av_guess_format("rtp", NULL, NULL);
  strcpy(rtpContext->filename, filename);
  rtpContext->flags = AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;

  if(!rtpContext->oformat) {
    avformat_free_context(rtpContext);
    cerr << "av_guess_format" << endl;
    exit(1);
  }

  if(!(rtpContext->oformat->flags & AVFMT_NOFILE)) {
    cout << "context's oformat is FILE" << endl;
    ret = avio_open2(&rtpContext->pb, rtpContext->filename, AVIO_FLAG_WRITE, NULL, NULL);
    if(ret < 0) {
      avformat_free_context(rtpContext);
      cerr << "avio_open2" << endl;
      exit(1);
    }
  }

  /*   2. set RTP stream */ cout << "==2==" << endl;
  rtpStream = addStream(rtpContext, &rtpCodec, codecName);

  /*   3. open codec & set frame */ cout << "==3==" << endl;
  ret = avcodec_open2(rtpStream->codec, rtpCodec, NULL);
  if(ret) {
    avformat_free_context(rtpContext);
    cerr << "avcodec_open2" << endl;
    exit(1);
  }
  rtpFrame = av_frame_alloc();
  rtpFrame->width = WIDTH;
  rtpFrame->height = HEIGHT;
  rtpFrame->format = static_cast<int>(rtpStream->codec->pix_fmt);
  vector<uint8_t> frameBuf(avpicture_get_size(rtpStream->codec->pix_fmt, WIDTH, HEIGHT));
  avpicture_fill(reinterpret_cast<AVPicture*>(rtpFrame), frameBuf.data(), rtpStream->codec->pix_fmt, WIDTH, HEIGHT);

  /*   4. init sample scaler */
  SwsContext *swsContext = sws_getCachedContext(NULL, WIDTH, HEIGHT, AV_PIX_FMT_RGB24, WIDTH, HEIGHT,
      rtpStream->codec->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);

  /*   5. write header */
  ret = avformat_write_header(rtpContext, NULL);
  if(ret < 0) {
    cerr << "avformat_write_header" << endl;
    exit(1);
  }
  char buf[200000];
  AVFormatContext *ac[] = { rtpContext };
  av_sdp_create(ac, 1, buf, 20000);
  printf("sdp:\n%s\n", buf);
  FILE* fsdp = fopen("test1.sdp", "w");
  fprintf(fsdp, "%s", buf);
  fclose(fsdp);

  /*   6. set & run encoding loop */
  int64_t framePts = 0;
  unsigned int nbFrames = 0;
  bool eos = false;
  int gotPkt = 0;
  do {
    // get webcam frame
    if(!eos) {
      cam >> cvFrame;
      imshow("press ESC to exit", cvFrame);
      if(waitKey(33) == 0x1b) {
        eos = true;
      }
    }

    // convert cvframe into ffmpeg frame
    if(!eos) {
      const int stride[] = {static_cast<int>(cvFrame.step[0])};
      sws_scale(swsContext, &cvFrame.data, stride, 0, cvFrame.rows, rtpFrame->data, rtpFrame->linesize);
      rtpFrame->pts = framePts++;
    }

    // encode video frame
    AVPacket packet;
    packet.data = nullptr;
    packet.size = 0;
    av_init_packet(&packet);
    ret = avcodec_encode_video2(rtpStream->codec, &packet, eos ? NULL : rtpFrame, &gotPkt);
    if(ret < 0) {
      cerr << "avcodec_encode_video2" << endl;
      break;
    }

    // Raw video case - directly store the picture in the packet
    if (rtpContext->oformat->flags & AVFMT_RAWPICTURE) {
      /*
      packet.flags        |= AV_PKT_FLAG_KEY;
      packet.stream_index  = rtpStream->index;
      packet.data          = dst_picture.data[0];
      packet.size          = sizeof(AVPicture);
      ret = av_interleaved_write_frame(oc, &pkt);*/
    }

    // send the encoded frame as packet
    if(gotPkt) {
      packet.pts = av_rescale_q_rnd(packet.pts, rtpStream->codec->time_base, rtpStream->time_base,
          AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
      packet.dts = av_rescale_q_rnd(packet.dts, rtpStream->codec->time_base, rtpStream->time_base,
          AVRounding(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
      packet.duration = av_rescale_q(packet.duration, rtpStream->codec->time_base, rtpStream->time_base);
      packet.stream_index = rtpStream->index;

      /* Write the compressed frame to the media file. */
      logPacket(rtpContext, &packet);
      av_interleaved_write_frame(rtpContext, &packet);
    }

  } while(!eos || gotPkt);

  /*   7. write any trailer before closing codec */
  av_write_trailer(rtpContext);

  /*   8. clean up */
  avcodec_close(rtpStream->codec);
  av_frame_free(&rtpFrame);
  if(!(rtpContext->oformat->flags & AVFMT_NOFILE))
    avio_close(rtpContext->pb);
  avformat_free_context(rtpContext);

  return 0;
}
