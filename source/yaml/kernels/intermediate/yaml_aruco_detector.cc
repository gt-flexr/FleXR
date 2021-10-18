#include "kernels/intermediate/aruco_detector.h"
#include <yaml/kernels/intermediate/yaml_aruco_detector.h>

namespace flexr
{
  namespace yaml
  {

    YamlArUcoDetector::YamlArUcoDetector(): YamlFleXRKernel()
    {
      width = height = 0;
      markerDict = "";
    }


    void YamlArUcoDetector::parseArUcoDetector(const YAML::Node &node)
    {
      parseBase(node);
      parseArUcoDetectorSpecific(node);
    }


    void YamlArUcoDetector::parseArUcoDetectorSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      markerDict        = others["marker_dict"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlArUcoDetector::printArUcoDetector()
    {
      printBase();
      printArUcoDetectorSpecific();
    }


    void YamlArUcoDetector::printArUcoDetectorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Dict: " << markerDict << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }


    void* YamlArUcoDetector::make()
    {
      if(baseSet && specificSet)
      {
        cv::aruco::PREDEFINED_DICTIONARY_NAME dictName = cv::aruco::DICT_6X6_250;
        if(markerDict == "DICT_6X6_250") dictName = cv::aruco::DICT_6X6_250;
        kernels::ArUcoDetector *temp = new kernels::ArUcoDetector(id, dictName, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::ArUcoDetectorInFrameType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::ArUcoDetectorInFrameType>(inPorts[i].portName,
                                                                              inPorts[i].protocol,
                                                                              inPorts[i].bindingPortNum);
            }
          }
          else debug_print("invalid input port_name %s for ArUcoDetector", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_marker_pose")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::ArUcoDetectorOutPoseType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::ArUcoDetectorOutPoseType>(outPorts[i].portName,
                                                                                outPorts[i].protocol,
                                                                                outPorts[i].connectingAddr,
                                                                                outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_marker_pose")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::ArUcoDetectorOutPoseType>(outPorts[i].duplicatedFrom,
                                                                                 outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::ArUcoDetectorOutPoseType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].protocol, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for ArUcoDetector", outPorts[i].portName.c_str());
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

