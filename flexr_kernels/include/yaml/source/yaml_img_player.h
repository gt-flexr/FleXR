#pragma once

#ifdef __FLEXR_KERNEL_IMG_PLAYER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/source/img_player.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml Bag camera kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * image_path     | Image path
     * filename_zeros | Zero padding to filename
     * num_of_images  | Number of images in the path
     * iterate        | iterating or not
     * caching        | initial frame caching or not
     */
    class YamlImgPlayer: public YamlFleXRKernel
    {
      public:
        std::string imgPath;
        int fnZeros, nImgs;
        bool iterate, caching;

        YamlImgPlayer();

        /**
         * @brief Parse ImgPlayer kernel info
         * @param node
         *  YAML node to parse
         */
        void parseImgPlayer(const YAML::Node &node);


        /**
         * @brief Parse specifics of ImgPlayer kernel
         * @param node
         *  YAML node to parse
         */
        void parseImgPlayerSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed ImgPlayer info
         */
        void printImgPlayer();


        /**
         * @brief Print parsed ImgPlayer specifics
         */
        void printImgPlayerSpecific();


        void* make();

    };
  }
}
#endif

