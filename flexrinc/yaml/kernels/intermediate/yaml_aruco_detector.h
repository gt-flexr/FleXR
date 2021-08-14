#ifndef __FLEXR_YAML_ARUCO_DETECTOR__
#define __FLEXR_YAML_ARUCO_DETECTOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml ArUcoDetector kernel
     * @see flexr::kernels::ArUcoDetector
     */
    class YamlArUcoDetector: public YamlFleXRKernel
    {
      public:
        int width, height;
        std::string markerDict;

        YamlArUcoDetector();

        /**
         * @brief Parse ArUcoDetector kernel info
         * @param node
         *  YAML node to parse
         */
        void parseArUcoDetector(const YAML::Node &node);


        /**
         * @brief Parse specifics of ArUcoDetector kernel
         * @param node
         *  YAML node to parse
         */
        void parseArUcoDetectorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed ArUcoDetector info
         */
        void printArUcoDetector();


        /**
         * @brief Print parsed ArUcoDetector specifics
         */
        void printArUcoDetectorSpecific();
    };
  }
}
#endif

