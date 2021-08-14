#include <yaml/yaml_flexr_kernel.h>

namespace flexr
{
  namespace yaml
  {
    YamlFleXRKernel::YamlFleXRKernel()
    {
      name = id = loggerId = loggerFileName = "";
      frequency = 0;
    }

    void YamlFleXRKernel::parseInPorts(const YAML::Node &node)
    {
      for(int i = 0; i < node.size(); i++) {
        YamlInPort inPort;
        inPort.portName       = node[i]["port_name"].as<std::string>();
        inPort.connectionType = node[i]["connection_type"].as<std::string>();
        if(inPort.connectionType == std::string("remote") && node[i]["binding_info"].IsDefined())
        {
          inPort.bindingPortNum = node[i]["binding_info"].as<int>();
        }
        inPorts.push_back(inPort);
      }
    }


    void YamlFleXRKernel::parseOutPorts(const YAML::Node &node)
    {
      for(int i = 0; i < node.size(); i++) {
        YamlOutPort outPort;
        outPort.portName          = node[i]["port_name"].as<std::string>();
        outPort.connectionType    = node[i]["connection_type"].as<std::string>();

        if(outPort.connectionType == std::string("remote") && node[i]["remote_info"].IsDefined())
        {
          outPort.connectingAddr    = node[i]["remote_info"][0].as<std::string>();
          outPort.connectingPortNum = node[i]["remote_info"][1].as<int>();
        }

        if(node[i]["duplicated_from"].IsDefined())
        {
          outPort.duplicatedFrom = node[i]["duplicated_from"].as<std::string>();
        }

        outPorts.push_back(outPort);
      }
    }


    void YamlFleXRKernel::printBase()
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


    void YamlFleXRKernel::parseBase(const YAML::Node &node)
    {
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

  }
}
