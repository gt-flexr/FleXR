#ifndef __FLEXR_YAML_CV_CAMERA__
#define __FLEXR_YAML_CV_CAMERA__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml CV camera kernel
     * @see flexr::kernels::CVCamera
     */
    class YamlCvCamera: public YamlFleXRKernel
    {
      public:
        int devIdx, width, height;

        YamlCvCamera();

        /**
         * @brief Parse CV camera kernel info
         * @param node
         *  YAML node to parse
         */
        void parseCvCamera(const YAML::Node &node);


        /**
         * @brief Parse specifics of CV camera kernel
         * @param node
         *  YAML node to parse
         */
        void parseCvCameraSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed CV camera info
         */
        void printCvCamera();


        /**
         * @brief Print parsed CV camera specifics
         */
        void printCvCameraSpecific();
    };
  }
}

#endif

