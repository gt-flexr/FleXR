#ifdef __FLEXR_KERNEL_RGBA2RGB_APP_SOURCE__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlRgba2RgbAppSource::YamlRgba2RgbAppSource(): YamlFleXRKernel()
    {
      shmqName = "";
      maxShmqElem = 2;
      width = height = 0;
    }


    void YamlRgba2RgbAppSource::parseRgba2RgbAppSource(const YAML::Node &node)
    {
      parseBase(node);
      parseRgba2RgbAppSourceSpecific(node);
    }


    void YamlRgba2RgbAppSource::parseRgba2RgbAppSourceSpecific(const YAML::Node &node)
    {
      specificSet = true;
      YAML::Node others = node["others"][0];
      shmqName    = others["shmq_name"].as<std::string>();
      maxShmqElem = others["max_shmq_elem"].as<int>();
      width       = others["width"].as<int>();
      height      = others["height"].as<int>();
    }


    void YamlRgba2RgbAppSource::printRgba2RgbAppSource()
    {
      printBase();
      printRgba2RgbAppSourceSpecific();
    }


    void YamlRgba2RgbAppSource::printRgba2RgbAppSourceSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tShmQueue Name" << shmqName << std::endl;
      std::cout << "\tMax eleme" << maxShmqElem << std::endl;
      std::cout << "\tFrame resolution " << width << " x " << height << std::endl;
    }


    void* YamlRgba2RgbAppSource::make()
    {
      if(baseSet && specificSet)
      {
        kernels::Rgba2RgbAppSource *temp = new kernels::Rgba2RgbAppSource(id, shmqName, maxShmqElem, width, height, frequency);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<types::Message<types::Frame>>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<types::Message<types::Frame>>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<types::Message<types::Frame>>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<types::Message<types::Frame>>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for Rgba2RgbAppSource", outPorts[i].portName.c_str());
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

