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
    using FrameReceiverMsgType = types::Message<types::Frame>;

    class RTPFrameReceiver : public MXREKernel
    {
      private:
        components::RTPReceiver rtpReceiver;
        // TODO: components::Decoder decoder;

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
        raft::kstatus run() override;
      void activateOutPortAsRemote(const std::string tag, int portNumber)
      {
        debug_print("not allow remote port activation.");
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

