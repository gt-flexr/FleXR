#ifndef __MXRE_RTP_RECEIVER__
#define __MXRE_RTP_RECEIVER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>
#include <zmq.hpp>

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
    class RTPFrameReceiver : public MXREKernel
    {
      private:
        int width, height;
        std::string decoder, sdp;

        // Frame Tracking
        zmq::context_t ctx;
        zmq::socket_t subscriber;

        // RTP Receiving Stream
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
        RTPFrameReceiver(std::string decoder, std::string srcAddr, int destPort, int width, int height);
        ~RTPFrameReceiver();
        void initRTPContext();
        void initRTPCodecAndScaler();
        void initFrame();
        void clearSession();
        void recvSDP(int srcPort);
        virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

