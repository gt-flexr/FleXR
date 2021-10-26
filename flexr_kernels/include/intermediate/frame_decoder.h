#pragma once

#ifdef __FLEXR_KERNEL_FRAME_DECODER__

#include <opencv2/opencv.hpp>

#include "flexr_core/include/core.h"

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
    using DecoderInEncodedFrameType = types::Message<uint8_t*>;
    using DecoderOutFrameType = types::Message<types::Frame>;


    /**
     * @brief Kernel to decode an encoded frame
     *
     * Port Tag         | Type
     * -----------------| ----------------------------
     * in_encoded_frame | @ref flexr::types::Message<uint8_t*>
     * out_frame        | @ref flexr::types::Message< @ref flexr::types::Frame>
     */
    class FrameDecoder : public FleXRKernel
    {
      private:
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
         * @param decoderName
         *  Decoder name to decode received frames
         * @param width
         *  Frame width
         * @param height
         *  Frame height
         * @see flexr::components::RTPReceiver
         */
        FrameDecoder(std::string id, std::string decoderName, int width, int height);


        ~FrameDecoder();


        raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

