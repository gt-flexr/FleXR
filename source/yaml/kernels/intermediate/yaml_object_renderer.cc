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
      specificSet       = true;
      YAML::Node others = node["others"][0];
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
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


    void* YamlObjectRenderer::make()
    {
      if(baseSet && specificSet)
      {
        kernels::ObjectRenderer *temp = new kernels::ObjectRenderer(id, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::ObjRendererInFrameType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::ObjRendererInFrameType>(inPorts[i].portName,
                                                                            inPorts[i].bindingPortNum);
            }
          }

          if(inPorts[i].portName == "in_marker_contexts")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::ObjRendererInCtxType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::ObjRendererInCtxType>(inPorts[i].portName,
                                                                          inPorts[i].bindingPortNum);
            }
          }

          if(inPorts[i].portName == "in_key")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::ObjRendererInKeyType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::ObjRendererInKeyType>(inPorts[i].portName,
                                                                          inPorts[i].bindingPortNum);
            }
          }
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
            {
              if(outPorts[i].duplicatedFrom == "")
                temp->activateOutPortAsLocal<kernels::ObjRendererOutFrameType>(outPorts[i].portName);
              else
                temp->duplicateOutPortAsLocal<kernels::ObjRendererOutFrameType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName);
            }
            else if(outPorts[i].connectionType == "remote")
            {
              if(outPorts[i].duplicatedFrom == "")
                temp->activateOutPortAsRemote<kernels::ObjRendererOutFrameType>(
                    outPorts[i].portName,
                    outPorts[i].connectingAddr,
                    outPorts[i].connectingPortNum);
              else
                temp->duplicateOutPortAsRemote<kernels::ObjRendererOutFrameType>(
                    outPorts[i].duplicatedFrom,
                    outPorts[i].portName,
                    outPorts[i].connectingAddr,
                    outPorts[i].connectingPortNum);
            }
          }
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

