#ifdef __FLEXR_KERNEL_RGB2RGBA_APP_SINK__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {
    YamlRgb2RgbaAppSink::YamlRgb2RgbaAppSink(): YamlFleXRKernel()
    {
      shmqName = "";
      maxShmqElem = 2;
      width = height = 0;
    }

    void YamlRgb2RgbaAppSink::parseRgb2RgbaAppSink(const YAML::Node &node)
    {
      parseBase(node);
      parseRgb2RgbaAppSinkSpecific(node);
    }


    void YamlRgb2RgbaAppSink::parseRgb2RgbaAppSinkSpecific(const YAML::Node &node)
    {
      specificSet = true;
      YAML::Node others = node["others"][0];
      shmqName    = others["shmq_name"].as<std::string>();
      maxShmqElem = others["max_shmq_elem"].as<int>();
      width       = others["width"].as<int>();
      height      = others["height"].as<int>();
    }


    void YamlRgb2RgbaAppSink::printRgb2RgbaAppSink()
    {
      printBase();
      printRgb2RgbaAppSinkSpecific();
    }


    void YamlRgb2RgbaAppSink::printRgb2RgbaAppSinkSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tShmQueue Name" << shmqName << std::endl;
      std::cout << "\tMax eleme" << maxShmqElem << std::endl;
      std::cout << "\tFrame resolution " << width << " x " << height << std::endl;
    }


    void* YamlRgb2RgbaAppSink::make()
    {
      if(baseSet && specificSet)
      {
        kernels::Rgb2RgbaAppSink *temp = new kernels::Rgb2RgbaAppSink(id, shmqName, maxShmqElem, width, height, frequency);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<types::Message<types::Frame>>(inPorts[i]);
            else
              temp->activateInPortAsRemote<types::Message<types::Frame>>(inPorts[i]);
          }
          else debug_print("invalid input port_name %s for Rgb2RgbaAppSink", inPorts[i].portName.c_str());
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

