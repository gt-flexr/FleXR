#ifndef __FLEXR_KERNEL_FRAME_ENCODER__
#define __FLEXR_KERNEL_FRAME_ENCODER__

#include <bits/stdc++.h>
#include <raft>

#include "flexr_core/include/core.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

namespace flexr
{
  namespace kernels
  {

    using EncoderInFrameType = types::Message<types::Frame>;
    using EncoderOutEncodedFrameType = types::Message<uint8_t*>;


    /**
     * @brief Kernel to encode a raw frame
     *
     * Port Tag          | Type
     * ------------------| ----------------------------
     * in_frame          | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_encoded_frame | encoded frame @ref flexr::types::Message<uint8_t*>
     */
    class FrameEncoder : public FleXRKernel
    {
      private:
        std::string encoderName;
        int width, height;

        AVCodec* encoder;
        AVCodecContext *encoderContext;
        AVFrame *encodingFrame;

        cv::Mat yuvFrame;

      public:
        /**
         * @brief Initialize RTP frame sender
         * @param id
         *  Kernel ID
         * @param encoderName
         *  Encoder name to encode sending frames
         * @param width
         *  Frame width
         * @param height
         *  Frame height
         * @param fps
         *  Target FPS of encoding w.r.t. bitrates
         * @see flexr::components::RTPSender
         */
        FrameEncoder(std::string id, std::string encoderName, int width, int height, int bitrate, int fps=60);


        ~FrameEncoder();


        raft::kstatus run() override;
    };

  } // namespace kernels
} // namespace flexr

#endif

