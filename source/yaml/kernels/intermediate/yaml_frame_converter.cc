#include <yaml/kernels/intermediate/yaml_frame_converter.h>

namespace flexr
{
  namespace yaml
  {

    YamlFrameConverter::YamlFrameConverter(): YamlFleXRKernel()
    {
      width = height = 0;
      conversionType = "";
    }


    void YamlFrameConverter::parseFrameConverter(const YAML::Node &node)
    {
      parseBase(node);
      parseFrameConverterSpecific(node);
    }


    void YamlFrameConverter::parseFrameConverterSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
      conversionType    = others["conversion_type"].as<std::string>();
    }


    void YamlFrameConverter::printFrameConverter()
    {
      printBase();
      printFrameConverterSpecific();
    }


    void YamlFrameConverter::printFrameConverterSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
      std::cout << "\tConversion Type: " << conversionType << std::endl;
    }


    void* YamlFrameConverter::make()
    {
      if(baseSet && specificSet)
      {
        kernels::FrameConverter *temp = new kernels::FrameConverter(id);

        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);
        temp->setConversion(conversionType);
        temp->setResolution(width, height);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].connectionType == "local")
          {
            temp->activateInPortAsLocal<kernels::FrameConverterInMsgType>(inPorts[i].portName);
          }
          else if(inPorts[i].connectionType == "remote")
          {
            temp->activateInPortAsRemote<kernels::FrameConverterInMsgType>(inPorts[i].portName,
                                                                          inPorts[i].bindingPortNum);
          }
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].connectionType == "local")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsLocal<kernels::FrameConverterOutMsgType>(outPorts[i].portName);
            else
              temp->duplicateOutPortAsLocal<kernels::FrameConverterOutMsgType>(outPorts[i].duplicatedFrom,
                                                                               outPorts[i].portName);
          }
          else if(outPorts[i].connectionType == "remote")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsRemote<kernels::FrameConverterOutMsgType>(
                  outPorts[i].portName,
                  outPorts[i].connectingAddr,
                  outPorts[i].connectingPortNum);
            else
              temp->duplicateOutPortAsRemote<kernels::FrameConverterOutMsgType>(
                  outPorts[i].duplicatedFrom,
                  outPorts[i].portName,
                  outPorts[i].connectingAddr,
                  outPorts[i].connectingPortNum);
          }
        }
        return temp;
      }
      else
      {
        debug_print("yaml recipe is invalid, fail to make.");
      }

      return nullptr;
    }

  }
}

