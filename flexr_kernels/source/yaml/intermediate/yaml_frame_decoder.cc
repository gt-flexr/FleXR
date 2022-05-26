#ifdef __FLEXR_KERNEL_FRAME_DECODER__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlFrameDecoder::YamlFrameDecoder(): YamlFleXRKernel()
    {
      decoder = "";
      width = height = 0;
    }


    void YamlFrameDecoder::parseFrameDecoder(const YAML::Node &node)
    {
      parseBase(node);
      parseFrameDecoderSpecific(node);
    }


    void YamlFrameDecoder::parseFrameDecoderSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      decoder           = others["decoder"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlFrameDecoder::printFrameDecoder()
    {
      printBase();
      printFrameDecoderSpecific();
    }


    void YamlFrameDecoder::printFrameDecoderSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tDecoder: " << decoder << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
    }


    void* YamlFrameDecoder::make()
    {
      if(baseSet && specificSet)
      {
        kernels::FrameDecoder *temp = new kernels::FrameDecoder(id, decoder, width, height);
        temp->setFrequency(frequency);
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
          else debug_print("invalid input port_name %s for FrameDecoder", inPorts[i].portName.c_str());
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

#endif

