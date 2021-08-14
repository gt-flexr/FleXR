#include <yaml/kernels/intermediate/yaml_aruco_detector.h>

namespace flexr
{
  namespace yaml
  {

    YamlArUcoDetector::YamlArUcoDetector(): YamlFleXRKernel()
    {
      width = height = 0;
      markerDict = "";
    }


    void YamlArUcoDetector::parseArUcoDetector(const YAML::Node &node)
    {
      parseBase(node);
      parseArUcoDetectorSpecific(node);
    }


    void YamlArUcoDetector::parseArUcoDetectorSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      markerDict        = others["marker_dict"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlArUcoDetector::printArUcoDetector()
    {
      printBase();
      printArUcoDetectorSpecific();
    }


    void YamlArUcoDetector::printArUcoDetectorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Dict: " << markerDict << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }

  }
}

