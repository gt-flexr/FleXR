#ifndef __FLEXR_YAML_FRAME_CONVERTER__
#define __FLEXR_YAML_FRAME_CONVERTER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/intermediate/frame_converter.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml FrameConverter kernel
     * @see flexr::kernels::FrameConverter
     */
    class YamlFrameConverter: public YamlFleXRKernel
    {
      public:
        int width, height;
        std::string conversionType;

        YamlFrameConverter();

        /**
         * @brief Parse FrameConverter kernel info
         * @param node
         *  YAML node to parse
         */
        void parseFrameConverter(const YAML::Node &node);


        /**
         * @brief Parse specifics of FrameConverter kernel
         * @param node
         *  YAML node to parse
         */
        void parseFrameConverterSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed FrameConverter info
         */
        void printFrameConverter();


        /**
         * @brief Print parsed FrameConverter specifics
         */
        void printFrameConverterSpecific();


        void* make();
    };
  }
}
#endif

