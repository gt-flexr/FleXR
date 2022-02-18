#pragma once

#ifdef __FLEXR_KERNEL_NVMPI_ENCODER__

#include "flexr_core/include/core.h"
#include <nvmpi_portable.h>


namespace flexr
{
  namespace kernels
  {
    using NvEncoderInFrameType = types::Message<types::Frame>;
    using NvEncoderOutEncodedFrameType = types::Message<uint8_t*>;


    /**
     * @brief Kernel to encode a raw frame with nvmpi
     *
     * Port Tag          | Type
     * ------------------| ----------------------------
     * in_frame          | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_encoded_frame | encoded frame @ref flexr::types::Message<uint8_t*>
     */
    class NvmpiEncoder : public FleXRKernel
    {
      private:
        std::string encoderName;
        int width, height;

        nvmpictx* encoder;
        cv::Mat yuvFrame;


      public:
        /**
         * @brief Initialize Nvmpi Encoder
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
         * @param QP
         *  Target QP of encoding
         */
        NvmpiEncoder(std::string id, std::string encoderName, int width, int height, int bitrate, int fps=60, int qp=10);


        ~NvmpiEncoder();


        raft::kstatus run() override;
    };

  } // namespace kernels
} // namespace flexr

#endif

