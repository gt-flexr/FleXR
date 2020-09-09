#include <iostream>
#include <vector>
// FFmpeg
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavformat/avio.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}
// OpenCV
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;


int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage: ff2cv <codec name>" << std::endl;
    return 1;
  }
  const char* codecName = argv[1];

  /* FFmpeg RTP Settings */
  const char *filename = "/home/jin/github/mxre/build/test.sdp";
  AVFormatContext *rtpContext;
  AVDictionary *whitelist = NULL;

  int videoStreamIndex;
  AVCodec *rtpCodec;
  AVCodecContext *rtpCodecContext;
  AVFrame *rtpFrame;
  int ret;

  av_register_all();
  avcodec_register_all();
  avformat_network_init();

  /*   1. set RTP context */ cout << "==1==" << endl;
  rtpContext = avformat_alloc_context();
  av_dict_set(&whitelist, "protocol_whitelist", "file,udp,rtp,crypto", 0);
  rtpContext->flags = AVFMT_FLAG_NOBUFFER | AVFMT_FLAG_FLUSH_PACKETS;

  ret = avformat_open_input(&rtpContext, filename, NULL, &whitelist);
  if(ret != 0) {
    avformat_free_context(rtpContext);
    cerr << "avformat_open_input" << endl;
    exit(1);
  }

  ret = avformat_find_stream_info(rtpContext, NULL);
  if(ret < 0) {
    avformat_free_context(rtpContext);
    cerr << "avformat_find_stream_info" << endl;
    exit(1);
  }
  for(unsigned int i = 0; i < rtpContext->nb_streams; i++) {
    if(rtpContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoStreamIndex = i;
      cout << "videoStreamIndex: " << videoStreamIndex << endl;
    }
  }
  av_read_play(rtpContext);

  rtpCodec = avcodec_find_decoder_by_name(codecName);
  if(rtpCodec == NULL) {
    avformat_free_context(rtpContext);
    cerr << "avcodec_find_decoder_by_name" << endl;
    exit(1);
  }
  rtpCodecContext = avcodec_alloc_context3(rtpCodec);
  avcodec_get_context_defaults3(rtpCodecContext, rtpCodec);
  avcodec_copy_context(rtpCodecContext, rtpContext->streams[videoStreamIndex]->codec);
  ret = avcodec_open2(rtpCodecContext, rtpCodec, NULL);
  if(ret < 0) {
    cerr << "avcodec_open2" << endl;
    exit(1);
  }
  if(strcmp(rtpCodecContext->codec->name, "h264_cuvid") == 0) {
    // ffmpeg -h decoder=h264_cuvid
    cout << "h264_cuvid" << endl;
    av_opt_set(rtpCodecContext->priv_data, "hwaccel", "cuvid", 0);
  }

  cout << rtpCodecContext->width << "x" << rtpCodecContext->height << endl;
  SwsContext *swsContext = sws_getContext(rtpCodecContext->width, rtpCodecContext->height, rtpCodecContext->pix_fmt,
      rtpCodecContext->width, rtpCodecContext->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

  int cvBufSize = avpicture_get_size(AV_PIX_FMT_RGB24, rtpCodecContext->width, rtpCodecContext->height);
  rtpFrame = av_frame_alloc();
  vector<uint8_t> cvFrameBuf(cvBufSize);
  avpicture_fill(reinterpret_cast<AVPicture*>(rtpFrame), cvFrameBuf.data(), AV_PIX_FMT_RGB24,
      rtpCodecContext->width, rtpCodecContext->height);

  int decBufSize = avpicture_get_size(AV_PIX_FMT_YUV420P, rtpCodecContext->width, rtpCodecContext->height);
  AVFrame *decFrame = av_frame_alloc();
  vector<uint8_t> decFrameBuf(decBufSize);
  avpicture_fill(reinterpret_cast<AVPicture*>(decFrame), decFrameBuf.data(), AV_PIX_FMT_YUV420P,
      rtpCodecContext->width, rtpCodecContext->height);

  int gotFrame = 0, readSuccess = -1;
  do {
    AVPacket packet;
    av_init_packet(&packet);
    readSuccess = av_read_frame(rtpContext, &packet);

    if(packet.stream_index == videoStreamIndex) {
      cout << "SAME" << endl;
      int result = avcodec_decode_video2(rtpCodecContext, decFrame, &gotFrame, &packet);
      cout << "AA" << endl;
      if(gotFrame) {
        std::cout << "Bytes decoded " << result << " check " << gotFrame << std::endl;
        sws_scale(swsContext, decFrame->data, decFrame->linesize, 0, decFrame->height,
            rtpFrame->data, rtpFrame->linesize);
        {
          cv::Mat image(rtpCodecContext->height, rtpCodecContext->width, CV_8UC3, cvFrameBuf.data(),
              rtpFrame->linesize[0]);
          cv::imshow("press ESC to exit", image);
          if (cv::waitKey(1) == 0x1b)
            break;
        }
      }
      cout << "AA" << endl;
    }
    av_free_packet(&packet);
  } while(readSuccess >= 0);

  /*   8. clean up */
  avcodec_close(rtpCodecContext);
  av_frame_free(&rtpFrame);
  av_frame_free(&decFrame);
  avformat_free_context(rtpContext);

  return 0;
}

