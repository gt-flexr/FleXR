#ifndef __MXRE_RTP_RECEIVER__
#define __MXRE_RTP_RECEIVER__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <string>
#include "defs.h"

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
      class RTPFrameReceiver : public raft::kernel
      {
        private:
          int width, height;
          std::string decoder, filename;

          AVFormatContext *rtpContext;
          AVDictionary *protocolWhitelist;
          int rtpStreamIndex;

          AVCodecContext *rtpCodecContext;
          AVCodec *rtpCodec;

          AVFrame *rtpFrame, *convertingFrame;
          std::vector<uint8_t> rtpFrameBuf, convertingFrameBuf;
          unsigned int rtpFrameSize, convertingFrameSize;

          SwsContext *swsContext;

        public:
          RTPFrameReceiver(std::string decoder, int width, int height);
          ~RTPFrameReceiver();
          void initRTPContext();
          void initRTPCodecAndScaler();
          void initFrame();
          void clearSession();
          virtual raft::kstatus run();
      };
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif

