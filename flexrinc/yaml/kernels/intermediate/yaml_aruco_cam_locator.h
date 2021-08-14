#ifndef __FLEXR_YAML_ARUCO_CAM_LOCATOR__
#define __FLEXR_YAML_ARUCO_CAM_LOCATOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml ArUcoCamLocator kernel
     * @see flexr::kernels::ArUcoCamLocator
     */
    class YamlArUcoCamLocator: public YamlFleXRKernel
    {
      public:
        int width, height;
        std::string markerDict;

        YamlArUcoCamLocator();

        /**
         * @brief Parse ArUcoCamLocator kernel info
         * @param node
         *  YAML node to parse
         */
        void parseArUcoCamLocator(const YAML::Node &node);


        /**
         * @brief Parse specifics of ArUcoCamLocator kernel
         * @param node
         *  YAML node to parse
         */
        void parseArUcoCamLocatorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed ArUcoCamLocator info
         */
        void printArUcoCamLocator();


        /**
         * @brief Print parsed ArUcoCamLocator specifics
         */
        void printArUcoCamLocatorSpecific();
    };
  }
}
#endif

