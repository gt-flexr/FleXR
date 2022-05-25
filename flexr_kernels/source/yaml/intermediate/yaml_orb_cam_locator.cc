#ifdef __FLEXR_KERNEL_ORB_CAM_LOCATOR__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlOrbCamLocator::YamlOrbCamLocator(): YamlFleXRKernel()
    {
      markerPath = "";
      width = height = 0;
    }


    void YamlOrbCamLocator::parseOrbCamLocator(const YAML::Node &node)
    {
      parseBase(node);
      parseOrbCamLocatorSpecific(node);
    }


    void YamlOrbCamLocator::parseOrbCamLocatorSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      markerPath        = others["marker_image"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlOrbCamLocator::printOrbCamLocator()
    {
      printBase();
      printOrbCamLocatorSpecific();
    }


    void YamlOrbCamLocator::printOrbCamLocatorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Path: " << markerPath << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }


    void* YamlOrbCamLocator::make()
    {
      if(baseSet && specificSet)
      {
        flexr::kernels::OrbCamLocator *temp = new kernels::OrbCamLocator(id, markerPath, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::OrbCamLocatorInFrame>(inPorts[i]);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::OrbCamLocatorInFrame>(inPorts[i]);
            }
          }
          else debug_print("invalid input port_name %s for OrbCamLocator", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_cam_pose")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::OrbCamLocatorOutPose>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::OrbCamLocatorOutPose>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_cam_pose")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::OrbCamLocatorOutPose>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::OrbCamLocatorOutPose>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for OrbCamLocator", outPorts[i].portName.c_str());
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

#endif

