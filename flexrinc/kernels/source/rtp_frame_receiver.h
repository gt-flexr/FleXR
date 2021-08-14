#ifndef __FLEXR_RTP_FRAME_RECEIVER__
#define __FLEXR_RTP_FRAME_RECEIVER__

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

namespace flexr
{
  namespace kernels
  {
    using FrameReceiverMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to receive RTP frame stream
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * out_frame      | @ref flexr::types::Message< @ref flexr::types::Frame>
     */
    class RTPFrameReceiver : public FleXRKernel
    {
      private:
        components::RTPReceiver rtpReceiver;

        std::string decoderName;
        int width, height;

        AVCodec *decoder;
        AVCodecContext *decoderContext;
        AVFrame *decodingFrame;
        uint8_t *decodingFrameBuffer;
        int decodingFrameSize;
        cv::Mat yuvFrame;


      public:
        /**
         * @brief Initialize RTP frame receiver
         * @param id
         *  Kernel ID
         * @param port
         *  Port number to receive the stream
         * @param decoderName
         *  Decoder name to decode received frames
         * @param width
         *  Frame width
         * @param height
         *  Frame height
         * @see flexr::components::RTPReceiver
         */
        RTPFrameReceiver(std::string id, int port, std::string decoderName, int width, int height);


        ~RTPFrameReceiver();


        raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

