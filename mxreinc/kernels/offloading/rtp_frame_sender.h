#ifndef __MXRE_RTP_FRAME_SENDER__
#define __MXRE_RTP_FRAME_SENDER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.hpp>
#include <ifaddrs.h>
#include <uvgrtp/lib.hh>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

namespace mxre
{
  namespace kernels
  {

    /* Class Deifinition */
    class RTPFrameSender : public MXREKernel
    {
      private:
        // Frame Tracking
        zmq::context_t ctx;
        zmq::socket_t publisher;

        // RTP Streaming
        uvg_rtp::context rtpContext;
        uvg_rtp::session *rtpSession;
        uvg_rtp::media_stream *rtpStream;

        // Encoder
        std::string encoderName;
        int width, height;

        AVCodec* encoder;
        AVCodecContext *encoderContext;
        AVFrame *encodingFrame;

        cv::Mat yuvFrame;

      public:
        RTPFrameSender(std::string destAddr, int destPortBase, std::string encoderName, int width, int height,
                       int bitrate, int fps=60);
        ~RTPFrameSender();
        virtual raft::kstatus run();
    };

  } // namespace kernels
} // namespace mxre

#endif

