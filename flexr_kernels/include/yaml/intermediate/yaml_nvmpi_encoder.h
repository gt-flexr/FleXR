#pragma once

#ifdef __FLEXR_KERNEL_NVMPI_ENCODER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/intermediate/nvmpi_encoder.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml NvmpiEncoder kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * encoder        | Encoder name
     * width          | Frmae width to send
     * height         | Frame height to send
     * bitrate        | Target bitrate
     * fps            | Target fps with br
     * qp             | Quantization factor
     *
     * @see flexr::kernels::NvmpiEncoder
     */
    class YamlNvmpiEncoder: public YamlFleXRKernel
    {
      public:
        std::string encoder;
        int width, height, bitrate, fps, qp;

        YamlNvmpiEncoder();

        /**
         * @brief Parse NvmpiEncoder kernel info
         * @param node
         *  YAML node to parse
         */
        void parseNvmpiEncoder(const YAML::Node &node);


        /**
         * @brief Parse specifics of NvmpiEncoder kernel
         * @param node
         *  YAML node to parse
         */
        void parseNvmpiEncoderSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed NvmpiEncoder info
         */
        void printNvmpiEncoder();


        /**
         * @brief Print parsed NvmpiEncoder specifics
         */
        void printNvmpiEncoderSpecific();


        void* make();
    };
  }
}

#endif

