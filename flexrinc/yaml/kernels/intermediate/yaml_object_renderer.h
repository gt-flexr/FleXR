#ifndef __FLEXR_YAML_OBJECT_RENDERER__
#define __FLEXR_YAML_OBJECT_RENDERER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/intermediate/object_renderer.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml ObjectRenderer kernel
     * @see flexr::kernels::ObjectRenderer
     */
    class YamlObjectRenderer: public YamlFleXRKernel
    {
      public:
        int width, height;

        YamlObjectRenderer();

        /**
         * @brief Parse ObjectRenderer kernel info
         * @param node
         *  YAML node to parse
         */
        void parseObjectRenderer(const YAML::Node &node);


        /**
         * @brief Parse specifics of ObjectRenderer kernel
         * @param node
         *  YAML node to parse
         */
        void parseObjectRendererSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed ObjectRenderer info
         */
        void printObjectRenderer();


        /**
         * @brief Print parsed ObjectRenderer specifics
         */
        void printObjectRendererSpecific();

        void* make();
    };
  }
}
#endif

