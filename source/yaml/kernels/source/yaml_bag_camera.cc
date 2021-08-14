#include <yaml/kernels/source/yaml_bag_camera.h>

namespace flexr
{
  namespace yaml
  {

    YamlBagCamera::YamlBagCamera(): YamlFleXRKernel()
    {
      framesToCache = startIndex = 0;
      bagPath = bagTopic = "";
    }


    void YamlBagCamera::parseBagCamera(const YAML::Node &node)
    {
      parseBase(node);
      parseBagCameraSpecific(node);
    }


    void YamlBagCamera::parseBagCameraSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      framesToCache     = others["frames_to_cache"].as<int>();
      startIndex        = others["start_index"].as<int>();
      bagPath           = others["bag_path"].as<std::string>();
      bagTopic          = others["bag_topic"].as<std::string>();
    }


    void YamlBagCamera::printBagCamera()
    {
      printBase();
      printBagCameraSpecific();
    }


    void YamlBagCamera::printBagCameraSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tCache frames, starting idx: " << framesToCache << ", " << startIndex << std::endl;
      std::cout << "\tBag File: " << bagPath << ", " << bagTopic << std::endl;
    }

  }
}
