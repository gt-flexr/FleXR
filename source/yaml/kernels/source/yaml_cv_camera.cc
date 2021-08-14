#include <yaml/kernels/source/yaml_cv_camera.h>

namespace flexr
{
  namespace yaml
  {

    YamlCvCamera::YamlCvCamera(): YamlFleXRKernel()
    {
      devIdx = width = height = 0;
    }


    void YamlCvCamera::parseCvCamera(const YAML::Node &node)
    {
      parseBase(node);
      parseCvCameraSpecific(node);
    }


    void YamlCvCamera::parseCvCameraSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      devIdx            = others["frames_to_cache"].as<int>();
      width             = others["start_index"].as<int>();
      height            = others["bag_path"].as<int>();
    }


    void YamlCvCamera::printCvCamera()
    {
      printBase();
      printCvCameraSpecific();
    }


    void YamlCvCamera::printCvCameraSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tCamera Device Index: " << devIdx << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
    }

  }
}
