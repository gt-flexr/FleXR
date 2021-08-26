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
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * frames_to_cache| Number of frames to cache
     * start_index    | Start frame index
     * bag_path       | Bag file path to read
     * bag_topic      | Bag file topic to subscribe
     *
     * @see flexr::kernels::BagCamera
     */
    class YamlBagCamera: public YamlFleXRKernel
    {
      public:
        int framesToCache, startIndex;
        std::string bagPath, bagTopic;

        YamlBagCamera();

        /**
         * @brief Parse BagCamera kernel info
         * @param node
         *  YAML node to parse
         */
        void parseBagCamera(const YAML::Node &node);


        /**
         * @brief Parse specifics of BagCamera kernel
         * @param node
         *  YAML node to parse
         */
        void parseBagCameraSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed BagCamera info
         */
        void printBagCamera();


        /**
         * @brief Print parsed BagCamera specifics
         */
        void printBagCameraSpecific();


        void* make();

    };
  }
}
#endif

