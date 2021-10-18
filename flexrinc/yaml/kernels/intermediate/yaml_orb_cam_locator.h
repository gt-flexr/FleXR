#ifndef __FLEXR_YAML_ORB_DETECTOR__
#define __FLEXR_YAML_ORB_DETECTOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/intermediate/orb_cam_locator.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml ORBCamLocator kernel
     * @see flexr::kernels::ORBCamLocator
     */
    class YamlOrbCamLocator: public YamlFleXRKernel
    {
      public:
        std::string markerPath;
        int width, height;

        YamlOrbCamLocator();

        /**
         * @brief Parse ORBCamLocator kernel info
         * @param node
         *  YAML node to parse
         */
        void parseOrbCamLocator(const YAML::Node &node);


        /**
         * @brief Parse specifics of ORBCamLocator kernel
         * @param node
         *  YAML node to parse
         */
        void parseOrbCamLocatorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed ORBCamLocator info
         */
        void printOrbCamLocator();


        /**
         * @brief Print parsed ORBCamLocator specifics
         */
        void printOrbCamLocatorSpecific();


        void* make();
    };
  }
}
#endif

