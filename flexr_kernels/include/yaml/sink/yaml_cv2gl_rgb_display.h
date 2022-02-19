#pragma once

#ifdef __FLEXR_KERNEL_CV2GL_RGB_DISPLAY__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/sink/cv2gl_rgb_display.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml Cv2GlRgbDisplay kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * width          | Frame width to display
     * height         | Frame height to display
     *
     * @see flexr::kernels::Cv2GlRgbDisplay
     */
    class YamlCv2GlRgbDisplay: public YamlFleXRKernel
    {
      public:
        int width, height;

        YamlCv2GlRgbDisplay();

        /**
         * @brief Parse Cv2GlRgbDisplay kernel info
         * @param node
         *  YAML node to parse
         */
        void parseCv2GlRgbDisplay(const YAML::Node &node);


        /**
         * @brief Parse specifics of Cv2GlRgbDisplay kernel
         * @param node
         *  YAML node to parse
         */
        void parseCv2GlRgbDisplaySpecific(const YAML::Node &node);


        /**
         * @brief Print parsed Cv2GlRgbDisplay info
         */
        void printCv2GlRgbDisplay();


        /**
         * @brief Print parsed Cv2GlRgbDisplay specifics
         */
        void printCv2GlRgbDisplaySpecific();


        void* make();
    };
  }
}
#endif

