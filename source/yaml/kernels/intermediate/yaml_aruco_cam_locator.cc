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
      specificSet       = true;
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


    void* YamlArUcoCamLocator::make()
    {
      if(baseSet && specificSet)
      {
        cv::aruco::PREDEFINED_DICTIONARY_NAME dictName = cv::aruco::DICT_6X6_250;
        if(markerDict == "DICT_6X6_250") dictName = cv::aruco::DICT_6X6_250;
        kernels::ArUcoCamLocator *temp = new kernels::ArUcoCamLocator(id, dictName, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::ArUcoCamLocatorInFrameType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::ArUcoCamLocatorInFrameType>(inPorts[i].portName,
                                                                                inPorts[i].bindingPortNum);
            }
          }
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].portName == "out_cam_pose")
          {
            if(outPorts[i].connectionType == "local")
            {
              if(outPorts[i].duplicatedFrom == "")
                temp->activateOutPortAsLocal<kernels::ArUcoCamLocatorOutPoseType>(outPorts[i].portName);
              else
                temp->duplicateOutPortAsLocal<kernels::ArUcoCamLocatorOutPoseType>(outPorts[i].duplicatedFrom,
                                                                                   outPorts[i].portName);
            }
            else if(outPorts[i].connectionType == "remote")
            {
              if(outPorts[i].duplicatedFrom == "")
                temp->activateOutPortAsRemote<kernels::ArUcoCamLocatorOutPoseType>(
                    outPorts[i].portName,
                    outPorts[i].connectingAddr,
                    outPorts[i].connectingPortNum);
              else
                temp->duplicateOutPortAsRemote<kernels::ArUcoCamLocatorOutPoseType>(
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

