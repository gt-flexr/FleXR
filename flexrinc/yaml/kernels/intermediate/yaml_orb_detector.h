#ifndef __FLEXR_YAML_ORB_DETECTOR__
#define __FLEXR_YAML_ORB_DETECTOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml ORBDetector kernel
     * @see flexr::kernels::ORBDetector
     */
    class YamlOrbDetector: public YamlFleXRKernel
    {
      public:
        std::string markerImage;

        YamlOrbDetector();

        /**
         * @brief Parse ORBDetector kernel info
         * @param node
         *  YAML node to parse
         */
        void parseOrbDetector(const YAML::Node &node);


        /**
         * @brief Parse specifics of ORBDetector kernel
         * @param node
         *  YAML node to parse
         */
        void parseOrbDetectorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed ORBDetector info
         */
        void printOrbDetector();


        /**
         * @brief Print parsed ORBDetector specifics
         */
        void printOrbDetectorSpecific();
    };
  }
}
#endif

