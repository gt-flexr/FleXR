#ifndef __FLEXR_YAML_BAG_CAMERA__
#define __FLEXR_YAML_BAG_CAMERA__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/source/bag_camera.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml Bag camera kernel
     * @see flexr::kernels::BagCamera
     */
    class YamlBagCamera: public YamlFleXRKernel
    {
      public:
        int framesToCache, startIndex;
        std::string bagPath, bagTopic;

        YamlBagCamera();

        /**
         * @brief Parse bag camera kernel info
         * @param node
         *  YAML node to parse
         */
        void parseBagCamera(const YAML::Node &node);


        /**
         * @brief Parse specifics of bag camera kernel
         * @param node
         *  YAML node to parse
         */
        void parseBagCameraSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed bag camera info
         */
        void printBagCamera();


        /**
         * @brief Print parsed bag camera specifics
         */
        void printBagCameraSpecific();


        void* make();

    };
  }
}
#endif

