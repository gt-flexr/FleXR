#ifndef __FLEXR_YAML_SAMPLE_MARKER_RENDERER__
#define __FLEXR_YAML_SAMPLE_MARKER_RENDERER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/intermediate/sample_marker_renderer.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml SampleMarkerRenderer kernel
     * @see flexr::kernels::SampleMarkerRenderer
     */
    class YamlSampleMarkerRenderer: public YamlFleXRKernel
    {
      public:
        int width, height;

        YamlSampleMarkerRenderer();

        /**
         * @brief Parse SampleMarkerRenderer kernel info
         * @param node
         *  YAML node to parse
         */
        void parseSampleMarkerRenderer(const YAML::Node &node);


        /**
         * @brief Parse specifics of SampleMarkerRenderer kernel
         * @param node
         *  YAML node to parse
         */
        void parseSampleMarkerRendererSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed SampleMarkerRenderer info
         */
        void printSampleMarkerRenderer();


        /**
         * @brief Print parsed SampleMarkerRenderer specifics
         */
        void printSampleMarkerRendererSpecific();


        void* make();
    };
  }
}
#endif

