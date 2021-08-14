#include <yaml/kernels/intermediate/yaml_marker_ctx_extractor.h>

namespace flexr
{
  namespace yaml
  {

    YamlMarkerCtxExtractor::YamlMarkerCtxExtractor(): YamlFleXRKernel()
    {
      width = height = 0;
    }


    void YamlMarkerCtxExtractor::parseMarkerCtxExtractor(const YAML::Node &node)
    {
      parseBase(node);
      parseMarkerCtxExtractorSpecific(node);
    }


    void YamlMarkerCtxExtractor::parseMarkerCtxExtractorSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      width = others["width"].as<int>();
      height = others["height"].as<int>();
    }


    void YamlMarkerCtxExtractor::printMarkerCtxExtractor()
    {
      printBase();
      printMarkerCtxExtractorSpecific();
    }


    void YamlMarkerCtxExtractor::printMarkerCtxExtractorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }

  }
}

