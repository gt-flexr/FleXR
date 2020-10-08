#ifndef __MXRE_RTP_SENDER__
#define __MXRE_RTP_SENDER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>
#include <zmq.h>
#include <ifaddrs.h>

#include "defs.h"
#include "cv_types.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      /* Class Deifinition */
      class RTPFrameSender : public raft::kernel
      {
        private:
          int bitrate, fps, width, height;
          int64_t framePts;
          std::string encoder, filename;

          AVFormatContext *rtpContext;
          AVStream *rtpStream;
          AVCodecContext *rtpCodecContext;
          AVCodec *rtpCodec;
          AVFrame *rtpFrame;
          SwsContext *swsContext;

        public:
          RTPFrameSender(std::string encoder, std::string destAddr, int destPort, int bitrate, int fps,
              int width, int height);
          ~RTPFrameSender();
          void setRTPContext();
          void setRTPStreamWithCodec();
          void setFrameWithScaler();
          void sendSDP(std::string &destAddr, int port);
          void clearSession();
          virtual raft::kstatus run();
      };
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif

