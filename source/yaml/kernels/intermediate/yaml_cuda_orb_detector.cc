#include <yaml/kernels/intermediate/yaml_cuda_orb_detector.h>

namespace flexr
{
  namespace yaml
  {

    YamlCudaOrbDetector::YamlCudaOrbDetector(): YamlFleXRKernel()
    {
      markerImage = "";
    }


    void YamlCudaOrbDetector::parseCudaOrbDetector(const YAML::Node &node)
    {
      parseBase(node);
      parseCudaOrbDetectorSpecific(node);
    }


    void YamlCudaOrbDetector::parseCudaOrbDetectorSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      markerImage       = others["marker_image"].as<std::string>();
    }


    void YamlCudaOrbDetector::printCudaOrbDetector()
    {
      printBase();
      printCudaOrbDetectorSpecific();
    }


    void YamlCudaOrbDetector::printCudaOrbDetectorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Image: " << markerImage << std::endl;
    }

  }
}

