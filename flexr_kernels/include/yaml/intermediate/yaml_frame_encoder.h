#pragma once

#ifdef __FLEXR_KERNEL_FRAME_ENCODER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/intermediate/frame_encoder.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml FrameEncoder kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * encoder        | Encoder name
     * width          | Frmae width to send
     * height         | Frame height to send
     * bitrate        | Target bitrate
     *
     * @see flexr::kernels::FrameEncoder
     */
    class YamlFrameEncoder: public YamlFleXRKernel
    {
      public:
        std::string encoder;
        int width, height, bitrate;

        YamlFrameEncoder();

        /**
         * @brief Parse FrameEncoder kernel info
         * @param node
         *  YAML node to parse
         */
        void parseFrameEncoder(const YAML::Node &node);


        /**
         * @brief Parse specifics of FrameEncoder kernel
         * @param node
         *  YAML node to parse
         */
        void parseFrameEncoderSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed FrameEncoder info
         */
        void printFrameEncoder();


        /**
         * @brief Print parsed FrameEncoder specifics
         */
        void printFrameEncoderSpecific();


        void* make();
    };
  }
}

#endif

