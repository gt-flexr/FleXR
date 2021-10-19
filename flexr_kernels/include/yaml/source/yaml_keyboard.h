#ifndef __FLEXR_KERNEL_YAML_KEYBOARD__
#define __FLEXR_KERNEL_YAML_KEYBOARD__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/source/keyboard.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml Keyboard kernel
     * @see flexr::kernels::Keyboard
     */
    class YamlKeyboard: public YamlFleXRKernel
    {
      public:
        int devIdx, width, height;


        YamlKeyboard();


        /**
         * @brief Parse Keyboard kernel info
         * @param node
         *  YAML node to parse
         */
        void parseKeyboard(const YAML::Node &node);


        /**
         * @brief Print parsed Keyboard info
         */
        void printKeyboard();


        void* make();
    };
  }
}

#endif

