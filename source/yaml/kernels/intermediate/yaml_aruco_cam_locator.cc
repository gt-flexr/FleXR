#include <yaml/kernels/intermediate/yaml_aruco_cam_locator.h>

namespace flexr
{
  namespace yaml
  {

    YamlArUcoCamLocator::YamlArUcoCamLocator(): YamlFleXRKernel()
    {
      width = height = 0;
      markerDict = "";
    }


    void YamlArUcoCamLocator::parseArUcoCamLocator(const YAML::Node &node)
    {
      parseBase(node);
      parseArUcoCamLocatorSpecific(node);
    }


    void YamlArUcoCamLocator::parseArUcoCamLocatorSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      markerDict        = others["marker_dict"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlArUcoCamLocator::printArUcoCamLocator()
    {
      printBase();
      printArUcoCamLocatorSpecific();
    }


    void YamlArUcoCamLocator::printArUcoCamLocatorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Dict: " << markerDict << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }

  }
}

