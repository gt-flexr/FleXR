#include <yaml/kernels/intermediate/yaml_sample_marker_renderer.h>

namespace flexr
{
  namespace yaml
  {

    YamlSampleMarkerRenderer::YamlSampleMarkerRenderer(): YamlFleXRKernel()
    {
      width = height = 0;
    }


    void YamlSampleMarkerRenderer::parseSampleMarkerRenderer(const YAML::Node &node)
    {
      parseBase(node);
      parseSampleMarkerRendererSpecific(node);
    }


    void YamlSampleMarkerRenderer::parseSampleMarkerRendererSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlSampleMarkerRenderer::printSampleMarkerRenderer()
    {
      printBase();
      printSampleMarkerRendererSpecific();
    }


    void YamlSampleMarkerRenderer::printSampleMarkerRendererSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }


    void* YamlSampleMarkerRenderer::make()
    {
      if(baseSet && specificSet)
      {
        kernels::SampleMarkerRenderer *temp = new kernels::SampleMarkerRenderer(id, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::SamMarRendFrame>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::SamMarRendFrame>(inPorts[i].portName,
                                                                     inPorts[i].protocol,
                                                                     inPorts[i].bindingPortNum);
            }
          }
          else if(inPorts[i].portName == "in_key")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::SamMarRendInKey>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::SamMarRendInKey>(inPorts[i].portName,
                                                                     inPorts[i].protocol,
                                                                     inPorts[i].bindingPortNum);
            }
          }
          else if(inPorts[i].portName == "in_cam_pose")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::SamMarRendInCamPose>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::SamMarRendInCamPose>(inPorts[i].portName,
                                                                         inPorts[i].protocol,
                                                                         inPorts[i].bindingPortNum);
            }
          }
          else debug_print("invalid input port_name %s for SampleMarkerRenderer", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::SamMarRendFrame>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::SamMarRendFrame>(outPorts[i].portName,
                                                                      outPorts[i].protocol,
                                                                      outPorts[i].connectingAddr,
                                                                      outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::SamMarRendFrame>(outPorts[i].duplicatedFrom,
                                                                        outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::SamMarRendFrame>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].protocol, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for SampleMarkerRenderer", outPorts[i].portName.c_str());
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

