#include <yaml/kernels/intermediate/yaml_orb_detector.h>

namespace flexr
{
  namespace yaml
  {

    YamlOrbDetector::YamlOrbDetector(): YamlFleXRKernel()
    {
      markerImage = "";
    }


    void YamlOrbDetector::parseOrbDetector(const YAML::Node &node)
    {
      parseBase(node);
      parseOrbDetectorSpecific(node);
    }


    void YamlOrbDetector::parseOrbDetectorSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      markerImage       = node["marker_image"].as<std::string>();
    }


    void YamlOrbDetector::printOrbDetector()
    {
      printBase();
      printOrbDetectorSpecific();
    }


    void YamlOrbDetector::printOrbDetectorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Image: " << markerImage << std::endl;
    }

  }
}

