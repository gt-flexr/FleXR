#include <yaml/kernels/intermediate/yaml_object_renderer.h>

namespace flexr
{
  namespace yaml
  {

    YamlObjectRenderer::YamlObjectRenderer(): YamlFleXRKernel()
    {
      width = height = 0;
    }


    void YamlObjectRenderer::parseObjectRenderer(const YAML::Node &node)
    {
      parseBase(node);
      parseObjectRendererSpecific(node);
    }


    void YamlObjectRenderer::parseObjectRendererSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      width = others["width"].as<int>();
      height = others["height"].as<int>();
    }


    void YamlObjectRenderer::printObjectRenderer()
    {
      printBase();
      printObjectRendererSpecific();
    }


    void YamlObjectRenderer::printObjectRendererSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tRendering Frame Resolution: " << width << "x" << height << std::endl;
    }

  }
}

