#ifdef __FLEXR_KERNEL_NVMPI_ENCODER__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlNvmpiEncoder::YamlNvmpiEncoder(): YamlFleXRKernel()
    {
      encoder = "";
      width = height = bitrate = fps = qp = 0;
    }


    void YamlNvmpiEncoder::parseNvmpiEncoder(const YAML::Node &node)
    {
      parseBase(node);
      parseNvmpiEncoderSpecific(node);
    }


    void YamlNvmpiEncoder::parseNvmpiEncoderSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      encoder           = others["encoder"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
      bitrate           = others["bitrate"].as<int>();
      fps               = others["fps"].as<int>();
      qp                = others["qp"].as<int>();
    }


    void YamlNvmpiEncoder::printNvmpiEncoder()
    {
      printBase();
      printNvmpiEncoderSpecific();
    }


    void YamlNvmpiEncoder::printNvmpiEncoderSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tEncoder: " << encoder << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
      std::cout << "\tEncoding Target Bitrate: " << bitrate << std::endl;
      std::cout << "\tEncoding FPS: " << fps << std::endl;
      std::cout << "\tEncoding QP: " << qp << std::endl;
    }


    void* YamlNvmpiEncoder::make()
    {
      if(baseSet && specificSet)
      {
        kernels::NvmpiEncoder *temp = new kernels::NvmpiEncoder(id, encoder, width, height, bitrate, fps, qp);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<kernels::EncoderInFrameType>(inPorts[i]);
            else if(inPorts[i].connectionType == "remote")
              temp->activateInPortAsRemote<kernels::EncoderInFrameType>(inPorts[i]);
          }
          else debug_print("invalid input port_name %s for NvmpiEncoder", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_encoded_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::EncoderOutEncodedFrameType>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::EncoderInFrameType>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_encoded_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::EncoderOutEncodedFrameType>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::EncoderOutEncodedFrameType>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for NvmpiEncoder", outPorts[i].portName.c_str());
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

