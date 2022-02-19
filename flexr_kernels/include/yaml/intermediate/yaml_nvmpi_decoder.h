#pragma once

#ifdef __FLEXR_KERNEL_NVMPI_DECODER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/intermediate/nvmpi_decoder.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml NvmpiDecoder kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * decoder        | Decoder name
     * width          | Frmae width to recv
     * height         | Frame height to recv
     *
     * @see flexr::kernels::NvmpiDecoder
     */
    class YamlNvmpiDecoder: public YamlFleXRKernel
    {
      public:
        std::string decoder;
        int width, height;

        YamlNvmpiDecoder();

        /**
         * @brief Parse NvmpiDecoder kernel info
         * @param node
         *  YAML node to parse
         */
        void parseNvmpiDecoder(const YAML::Node &node);


        /**
         * @brief Parse specifics of NvmpiDecoder kernel
         * @param node
         *  YAML node to parse
         */
        void parseNvmpiDecoderSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed NvmpiDecoder info
         */
        void printNvmpiDecoder();


        /**
         * @brief Print parsed NvmpiDecoder specifics
         */
        void printNvmpiDecoderSpecific();


        void* make();
    };
  }
}

#endif

