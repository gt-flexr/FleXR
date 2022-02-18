#pragma once

#ifdef __FLEXR_KERNEL_NVMPI_DECODER__

#include "flexr_core/include/core.h"
#include <nvmpi_portable.h>


namespace flexr
{
  namespace kernels
  {
    using NvDecoderInEncodedFrameType = types::Message<uint8_t*>;
    using NvDecoderOutFrameType       = types::Message<types::Frame>;


    /**
     * @brief Kernel to decode an encoded frame with nvmpi
     *
     * Port Tag         | Type
     * -----------------| ----------------------------
     * in_encoded_frame | @ref flexr::types::Message<uint8_t*>
     * out_frame        | @ref flexr::types::Message< @ref flexr::types::Frame>
     */
    class NvmpiDecoder : public FleXRKernel
    {
      private:
        std::string decoderName;
        int width, height;

        nvmpictx *decoder;
        uint8_t *decodingFrameBuffer;
        int decodingFrameSize;

      public:
        /**
         * @brief Initialize Nvmpi Decoder
         * @param id
         *  Kernel ID
         * @param decoderName
         *  Decoder name to decode received frames
         * @param width
         *  Frame width
         * @param height
         *  Frame height
         */
        NvmpiDecoder(std::string id, std::string decoderName, int width, int height);


        ~NvmpiDecoder();


        raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

