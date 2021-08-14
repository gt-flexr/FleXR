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

  }
}

