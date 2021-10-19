#ifndef __FLEXR_KERNEL_YAML_CV_DISPLAY__
#define __FLEXR_KERNEL_YAML_CV_DISPLAY__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/sink/cv_display.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml CVDisplay kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * No specifics   | NA
     *
     * @see flexr::kernels::CVDisplay
     */
    class YamlCvDisplay: public YamlFleXRKernel
    {
      public:
        YamlCvDisplay();

        /**
         * @brief Parse CVDisplay kernel info
         * @param node
         *  YAML node to parse
         */
        void parseCvDisplay(const YAML::Node &node);


        /**
         * @brief Print parsed CVDisplay info
         */
        void printCvDisplay();


        void* make();
    };
  }
}
#endif

