#ifndef __FLEXR_YAML_CV_DISPLAY__
#define __FLEXR_YAML_CV_DISPLAY__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/sink/cv_display.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml CVDisplay kernel
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

