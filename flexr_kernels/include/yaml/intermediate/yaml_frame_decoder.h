#ifndef __FLEXR_KERNEL_YAML_FRAME_DECODER__
#define __FLEXR_KERNEL_YAML_FRAME_DECODER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/intermediate/frame_decoder.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml FrameDecoder kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * decoder        | Decoder name
     * width          | Frmae width to send
     * height         | Frame height to send
     *
     * @see flexr::kernels::FrameDecoder
     */
    class YamlFrameDecoder: public YamlFleXRKernel
    {
      public:
        std::string decoder;
        int width, height;

        YamlFrameDecoder();

        /**
         * @brief Parse RTPFrameReceiver kernel info
         * @param node
         *  YAML node to parse
         */
        void parseFrameDecoder(const YAML::Node &node);


        /**
         * @brief Parse specifics of RTPFrameReceiver kernel
         * @param node
         *  YAML node to parse
         */
        void parseFrameDecoderSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed RTPFrameReceiver info
         */
        void printFrameDecoder();


        /**
         * @brief Print parsed RTPFrameReceiver specifics
         */
        void printFrameDecoderSpecific();


        void* make();
    };
  }
}

#endif

