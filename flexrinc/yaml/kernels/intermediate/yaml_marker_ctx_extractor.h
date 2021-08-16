#ifndef __FLEXR_YAML_MARKER_CTX_EXTRACTOR__
#define __FLEXR_YAML_MARKER_CTX_EXTRACTOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/intermediate/marker_ctx_extractor.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml MarkerCtxExtractor kernel
     * @see flexr::kernels::MarkerCtxExtractor
     */
    class YamlMarkerCtxExtractor: public YamlFleXRKernel
    {
      public:
        int width, height;

        YamlMarkerCtxExtractor();

        /**
         * @brief Parse MarkerCtxExtractor kernel info
         * @param node
         *  YAML node to parse
         */
        void parseMarkerCtxExtractor(const YAML::Node &node);


        /**
         * @brief Parse specifics of MarkerCtxExtractor kernel
         * @param node
         *  YAML node to parse
         */
        void parseMarkerCtxExtractorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed MarkerCtxExtractor info
         */
        void printMarkerCtxExtractor();


        /**
         * @brief Print parsed MarkerCtxExtractor specifics
         */
        void printMarkerCtxExtractorSpecific();


        void* make();
    };
  }
}
#endif

