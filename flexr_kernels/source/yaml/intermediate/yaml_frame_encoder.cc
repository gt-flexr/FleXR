#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlFrameEncoder::YamlFrameEncoder(): YamlFleXRKernel()
    {
      encoder = "";
      width = height = bitrate = 0;
    }


    void YamlFrameEncoder::parseFrameEncoder(const YAML::Node &node)
    {
      parseBase(node);
      parseFrameEncoderSpecific(node);
    }


    void YamlFrameEncoder::parseFrameEncoderSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      encoder           = others["encoder"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
      bitrate           = others["bitrate"].as<int>();
    }


    void YamlFrameEncoder::printFrameEncoder()
    {
      printBase();
      printFrameEncoderSpecific();
    }


    void YamlFrameEncoder::printFrameEncoderSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tEncoder: " << encoder << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
      std::cout << "\tEncoding Target Bitrate: " << bitrate << std::endl;
    }


    void* YamlFrameEncoder::make()
    {
      if(baseSet && specificSet)
      {
        kernels::FrameEncoder *temp = new kernels::FrameEncoder(id, encoder, width, height, bitrate, frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<kernels::EncoderInFrameType>(inPorts[i].portName);
            else if(inPorts[i].connectionType == "remote")
              temp->activateInPortAsRemote<kernels::EncoderInFrameType>(inPorts[i].portName,
                                                                        inPorts[i].protocol,
                                                                        inPorts[i].bindingPortNum);
          }
          else debug_print("invalid input port_name %s for FrameEncoder", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_encoded_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::EncoderOutEncodedFrameType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::EncoderInFrameType>(outPorts[i].portName,
                                                                         outPorts[i].protocol,
                                                                         outPorts[i].connectingAddr,
                                                                         outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_encoded_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::EncoderOutEncodedFrameType>(outPorts[i].duplicatedFrom,
                                                                                   outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::EncoderOutEncodedFrameType>(outPorts[i].duplicatedFrom,
                                                                                    outPorts[i].portName,
                                                                                    outPorts[i].protocol,
                                                                                    outPorts[i].connectingAddr,
                                                                                    outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for FrameDecoder", outPorts[i].portName.c_str());
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

