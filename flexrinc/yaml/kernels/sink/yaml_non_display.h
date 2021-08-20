#ifndef __FLEXR_YAML_NON_DISPLAY__
#define __FLEXR_YAML_NON_DISPLAY__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/sink/non_display.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml NonDisplay kernel
     * @see flexr::kernels::NonDisplay
     */
    class YamlNonDisplay: public YamlFleXRKernel
    {
      public:
        YamlNonDisplay();

        /**
         * @brief Parse NonDisplay kernel info
         * @param node
         *  YAML node to parse
         */
        void parseNonDisplay(const YAML::Node &node);


        /**
         * @brief Print parsed NonDisplay info
         */
        void printNonDisplay();


        void* make();
    };
  }
}
#endif

