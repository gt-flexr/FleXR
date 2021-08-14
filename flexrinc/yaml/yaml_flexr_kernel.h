
#ifndef __FLEXR_YAML_KERNEL__
#define __FLEXR_YAML_KERNEL__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "yaml_ports.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml FleXR kernel
     */
    class YamlFleXRKernel
    {
      protected:
        /**
         * @brief Parse input ports info of the base
         * @param node
         *  YAML node to parse
         */
        void parseInPorts(const YAML::Node &node);


        /**
         * @brief Parse output ports info of the base
         * @param node
         *  YAML node to parse
         */
        void parseOutPorts(const YAML::Node &node);


      public:
        std::string name, id;
        int frequency;
        std::string loggerId, loggerFileName;
        std::vector<YamlInPort> inPorts;
        std::vector<YamlOutPort> outPorts;

        YamlFleXRKernel();

        /**
         * @brief Print FleXR Kernel info
         */
        void printBase();


        /**
         * @brief Parse base info
         * @param node
         *  YAML node to parse
         */
        void parseBase(const YAML::Node &node);
    };
  }
}
#endif

