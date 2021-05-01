#ifndef __MXRE_RTP_FRAME_RECEIVER__
#define __MXRE_RTP_FRAME_RECEIVER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "components/rtp_receiver.h"

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
        // RTP Streaming
        components::RTPReceiver rtpReceiver;

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
        RTPFrameReceiver(int port, std::string decoderName, int width, int height);
        ~RTPFrameReceiver();
        virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

