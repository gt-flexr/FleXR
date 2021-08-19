#include <yaml/kernels/intermediate/yaml_orb_detector.h>

namespace flexr
{
  namespace yaml
  {

    YamlOrbDetector::YamlOrbDetector(): YamlFleXRKernel()
    {
      markerImage = "";
    }


    void YamlOrbDetector::parseOrbDetector(const YAML::Node &node)
    {
      parseBase(node);
      parseOrbDetectorSpecific(node);
    }


    void YamlOrbDetector::parseOrbDetectorSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      markerImage       = node["marker_image"].as<std::string>();
    }


    void YamlOrbDetector::printOrbDetector()
    {
      printBase();
      printOrbDetectorSpecific();
    }


    void YamlOrbDetector::printOrbDetectorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tMarker Image: " << markerImage << std::endl;
    }


    void* YamlOrbDetector::make()
    {
      if(baseSet && specificSet)
      {
        flexr::kernels::ORBDetector *temp = new kernels::ORBDetector(id, markerImage);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::ORBDetectorInFrameType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::ORBDetectorInFrameType>(inPorts[i].portName,
                  inPorts[i].bindingPortNum);
            }
          }
          else debug_print("invalid input port_name %s for ORBDetector", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_detected_markers")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::ORBDetectorOutMarkerType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::ORBDetectorOutMarkerType>(outPorts[i].portName,
                                                                               outPorts[i].connectingAddr,
                                                                               outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_detected_markers")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::ORBDetectorOutMarkerType>(outPorts[i].duplicatedFrom,
                                                                                 outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::ORBDetectorOutMarkerType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for ORBDetector", outPorts[i].portName.c_str());
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

