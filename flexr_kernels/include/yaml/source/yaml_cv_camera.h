#pragma once

#ifdef __FLEXR_KERNEL_CV_CAMERA__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/source/cv_camera.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml CVCamera kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * dev_index      | VideoCapture index
     * file_name      | VideoCapture file name
     * width          | Frame width to read
     * height         | Frame height to read
     *
     * @see flexr::kernels::CVCamera
     */
    class YamlCvCamera: public YamlFleXRKernel
    {
      public:
        int devIdx, width, height;
        std::string fileName;

        YamlCvCamera();

        /**
         * @brief Parse CVCamera kernel info
         * @param node
         *  YAML node to parse
         */
        void parseCvCamera(const YAML::Node &node);


        /**
         * @brief Parse specifics of CVCamera kernel
         * @param node
         *  YAML node to parse
         */
        void parseCvCameraSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed CVCamera info
         */
        void printCvCamera();


        /**
         * @brief Print parsed CVCamera specifics
         */
        void printCvCameraSpecific();


        void* make();
    };
  }
}

#endif

