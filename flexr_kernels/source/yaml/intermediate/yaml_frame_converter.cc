#ifdef __FLEXR_KERNEL_FRAME_CONVERTER__

#include <flexr_kernels/include/kernels.h>

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
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::FrameConverterInMsgType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::FrameConverterInMsgType>(inPorts[i].portName,
                                                                             inPorts[i].protocol,
                                                                             inPorts[i].bindingPortNum);
            }
          }
          else debug_print("invalid input port_name %s for FrameConverter", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::FrameConverterOutMsgType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::FrameConverterOutMsgType>(outPorts[i].portName,
                                                                               outPorts[i].protocol,
                                                                               outPorts[i].connectingAddr,
                                                                               outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::FrameConverterOutMsgType>(outPorts[i].duplicatedFrom,
                                                                                 outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::FrameConverterOutMsgType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].protocol, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for FrameConverter", outPorts[i].portName.c_str());
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

#endif

