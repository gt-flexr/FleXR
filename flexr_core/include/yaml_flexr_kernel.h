#ifndef __FLEXR_CORE_YAML_KERNEL__
#define __FLEXR_CORE_YAML_KERNEL__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "flexr_core/include/defs.h"
#include "flexr_core/include/yaml_ports.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml FleXR kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * kernel         | Kernel class
     * id             | ID for a kernel instance
     * frequency      | Target frequency of kernel
     * logger         | Logger info [id, log_file.log]
     * input          | @see flexr::yaml::YamlInPort
     * output         | @see flexr::yaml::YamlOutPort
     * others         | each kernel's yaml class specifics
     * @see flexr::kernels::FleXRKernel
     */
    class YamlFleXRKernel
    {
      protected:
        bool baseSet, specificSet;

        /**
         * @brief Parse input ports info of the base kernel
         * @param node
         *  YAML node to parse
         */
        void parseInPorts(const YAML::Node &node)
        {
          for(int i = 0; i < node.size(); i++) {
            YamlInPort inPort;
            inPort.portName       = node[i]["port_name"].as<std::string>();
            inPort.connectionType = node[i]["connection_type"].as<std::string>();
            if(inPort.connectionType == std::string("remote") && node[i]["remote_info"].IsDefined())
            {
              inPort.protocol       = node[i]["remote_info"][0].as<std::string>();
              inPort.bindingPortNum = node[i]["remote_info"][1].as<int>();
            }
            inPorts.push_back(inPort);
          }
        }


        /**
         * @brief Parse output ports info of the base kernel
         * @param node
         *  YAML node to parse
         */
        void parseOutPorts(const YAML::Node &node)
        {
          for(int i = 0; i < node.size(); i++) {
            YamlOutPort outPort;
            outPort.portName          = node[i]["port_name"].as<std::string>();
            outPort.connectionType    = node[i]["connection_type"].as<std::string>();

            if(outPort.connectionType == std::string("remote") && node[i]["remote_info"].IsDefined())
            {
              outPort.protocol          = node[i]["remote_info"][0].as<std::string>();
              outPort.connectingAddr    = node[i]["remote_info"][1].as<std::string>();
              outPort.connectingPortNum = node[i]["remote_info"][2].as<int>();
            }

            if(node[i]["duplicated_from"].IsDefined())
            {
              outPort.duplicatedFrom = node[i]["duplicated_from"].as<std::string>();
            }

            outPorts.push_back(outPort);
          }
        }


      public:
        std::string name, id;
        int frequency;
        std::string loggerId, loggerFileName;
        std::vector<YamlInPort> inPorts;
        std::vector<YamlOutPort> outPorts;

        YamlFleXRKernel()
        {
          baseSet = specificSet = false;
          name = id = loggerId = loggerFileName = "";
          frequency = 0;
        }


        /**
         * @brief Print base kernel info
         */
        void printBase()
        {
          std::cout << "FleXR Kernel Base -------- " << std::endl;
          std::cout << "\tKernel Name, ID: " << name << ", " << id << std::endl;
          std::cout << "\tFrequency: " << frequency << std::endl;
          std::cout << "\tLogger Info: " << loggerId << ", " << loggerFileName << std::endl;

          for(int i = 0; i < inPorts.size(); i++)
          {
            inPorts[i].print();
          }

          for(int i = 0; i < outPorts.size(); i++)
          {
            outPorts[i].print();
          }
        }




        /**
         * @brief Parse base kernel info
         * @param node
         *  YAML node to parse
         */
        void parseBase(const YAML::Node &node)
        {
          baseSet        = true;
          name           = node["kernel"].as<std::string>();
          id             = node["id"].as<std::string>();
          frequency      = node["frequency"].as<int>();
          loggerId       = node["logger"][0].as<std::string>();
          loggerFileName = node["logger"][1].as<std::string>();

          if(node["input"].IsDefined())
          {
            parseInPorts(node["input"]);
          }

          if(node["output"].IsDefined())
          {
            parseOutPorts(node["output"]);
          }
        }



        /**
         * @brief Make kernel instance with parsed yaml recipe
         * @return Pointer to the kernel instance
         */
        virtual void* make() {return nullptr;};
    };
  }
}
#endif

