#ifdef __FLEXR_KERNEL_NVMPI_DECODER__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlNvmpiDecoder::YamlNvmpiDecoder(): YamlFleXRKernel()
    {
      decoder = "";
      width = height = 0;
    }


    void YamlNvmpiDecoder::parseNvmpiDecoder(const YAML::Node &node)
    {
      parseBase(node);
      parseNvmpiDecoderSpecific(node);
    }


    void YamlNvmpiDecoder::parseNvmpiDecoderSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      decoder           = others["decoder"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlNvmpiDecoder::printNvmpiDecoder()
    {
      printBase();
      printNvmpiDecoderSpecific();
    }


    void YamlNvmpiDecoder::printNvmpiDecoderSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tDecoder: " << decoder << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
    }


    void* YamlNvmpiDecoder::make()
    {
      if(baseSet && specificSet)
      {
        kernels::NvmpiDecoder *temp = new kernels::NvmpiDecoder(id, decoder, width, height);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_encoded_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::DecoderInEncodedFrameType>(inPorts[i]);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::DecoderInEncodedFrameType>(inPorts[i]);
            }
          }
          else debug_print("invalid input port_name %s for NvmpiDecoder", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::DecoderOutFrameType>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::DecoderOutFrameType>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::DecoderOutFrameType>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::DecoderOutFrameType>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for NvmpiDecoder", outPorts[i].portName.c_str());
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

