#ifndef __MXRE_RTP_FRAME_RECEIVER__
#define __MXRE_RTP_FRAME_RECEIVER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>
#include <uvgrtp/lib.hh>
#include <zmq.hpp>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
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
    class RTPFrameReceiver : public MXREKernel
    {
      private:
        // Frame Tracking
        zmq::context_t ctx;
        zmq::socket_t subscriber;

        // RTP Streaming
        uvg_rtp::context rtpContext;
        uvg_rtp::session *rtpSession;
        uvg_rtp::media_stream *rtpStream;
        uvg_rtp::frame::rtp_frame *rtpFrame;

        // Decoder
        std::string decoderName;
        int width, height;

        AVCodec *decoder;
        AVCodecContext *decoderContext;
        AVFrame *decodingFrame;
        uint8_t *decodingFrameBuffer;
        int decodingFrameSize;
        cv::Mat yuvFrame;


      public:
        RTPFrameReceiver(int portBase, std::string decoderName, int width, int height);
        ~RTPFrameReceiver();
        virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

