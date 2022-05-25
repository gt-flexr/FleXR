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

    class YamlFleXRKernel
    {
      protected:
        bool baseSet, specificSet;


        void parseInPorts(const YAML::Node &node)
        {
          for(int i = 0; i < node.size(); i++) {
            YamlInPort inPort;

            inPort.portName       = node[i]["port_name"].as<std::string>();
            inPort.connectionType = node[i]["connection_type"].as<std::string>();

            // Connection Type -- remote
            if(inPort.connectionType == std::string("remote") && node[i]["remote_info"].IsDefined())
            {
              inPort.protocol       = node[i]["remote_info"][0].as<std::string>();
              inPort.bindingPortNum = node[i]["remote_info"][1].as<int>();
            }
            // Connection Type -- local
            else if(inPort.connectionType == std::string("local"))
            {
              inPort.localChannel = "raftlib"; // default -- raftlib

              if(node[i]["local_channel"].IsDefined() && node[i]["local_channel"][0].IsDefined())
              {
                // TODO: node checks
                // shm -- [shm, SHM_ID, SHM_SIZE, SHM_ELEM_SIZE]
                if(node[i]["local_channel"][0].as<std::string>() == std::string("shm"))
                {
                  inPort.localChannel = node[i]["local_channel"][0].as<std::string>();
                  inPort.shmId        = node[i]["local_channel"][1].as<std::string>();
                  inPort.shmSize      = node[i]["local_channel"][2].as<int>();
                  inPort.shmElemSize  = node[i]["local_channel"][3].as<int>();
                }
              }
            }
            inPorts.push_back(inPort);
          }
        }


        void parseOutPorts(const YAML::Node &node)
        {
          for(int i = 0; i < node.size(); i++) {
            YamlOutPort outPort;
            outPort.portName          = node[i]["port_name"].as<std::string>();
            outPort.connectionType    = node[i]["connection_type"].as<std::string>();

            // OutPort Semantics -- blocking or nonblocking
            if(outPort.connectionType == std::string("local") && node[i]["semantics"].IsDefined())
            {
              outPort.semantics = node[i]["semantics"].as<std::string>();
            }

            // Connection Type -- remote
            if(outPort.connectionType == std::string("remote") && node[i]["remote_info"].IsDefined())
            {
              outPort.protocol          = node[i]["remote_info"][0].as<std::string>();
              outPort.connectingAddr    = node[i]["remote_info"][1].as<std::string>();
              outPort.connectingPortNum = node[i]["remote_info"][2].as<int>();
            }
            // Connection Type -- local
            else if(outPort.connectionType == std::string("local"))
            {
              outPort.localChannel = "raftlib"; // default -- raftlib

              if(node[i]["local_channel"].IsDefined() && node[i]["local_channel"][0].IsDefined())
              {
                // TODO: node checks
                // shm -- [shm, SHM_ID, SHM_SIZE, SHM_ELEM_SIZE]
                if(node[i]["local_channel"][0].as<std::string>() == std::string("shm"))
                {
                  outPort.localChannel = node[i]["local_channel"][0].as<std::string>();
                  outPort.shmId        = node[i]["local_channel"][1].as<std::string>();
                  outPort.shmSize      = node[i]["local_channel"][2].as<int>();
                  outPort.shmElemSize  = node[i]["local_channel"][3].as<int>();
                }
              }
            }

            // OutPort Duplication
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


        virtual void* make() {return nullptr;};
    };
  }
}
#endif

