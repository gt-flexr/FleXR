#ifndef __MXRE_RTP_SENDER__
#define __MXRE_RTP_SENDER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>
#include <zmq.hpp>
#include <ifaddrs.h>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
}

namespace mxre
{
  namespace kernels
  {

    /* Class Deifinition */
    class RTPFrameSender : public MXREKernel
    {
      private:
        int bitrate, fps, width, height;
        int64_t framePts;
        std::string encoder, filename;

        // Frame Tracking
        zmq::context_t ctx;
        zmq::socket_t publisher;

        // RTP Sending Stream
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

  } // namespace kernels
} // namespace mxre

#endif

