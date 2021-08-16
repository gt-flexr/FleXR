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
      specificSet       = true;
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


    void* YamlCudaOrbDetector::make()
    {
      if(baseSet && specificSet)
      {
        kernels::CudaORBDetector *temp = new kernels::CudaORBDetector(id, markerImage);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::CudaORBDetectorInFrameType>(inPorts[i].portName);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::CudaORBDetectorInFrameType>(inPorts[i].portName,
                  inPorts[i].bindingPortNum);
            }
          }
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].portName == "out_detected_markers")
          {
            if(outPorts[i].connectionType == "local")
            {
              if(outPorts[i].duplicatedFrom == "")
                temp->activateOutPortAsLocal<kernels::CudaORBDetectorOutMarkerType>(outPorts[i].portName);
              else
                temp->duplicateOutPortAsLocal<kernels::CudaORBDetectorOutMarkerType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName);
            }
            else if(outPorts[i].connectionType == "remote")
            {
              if(outPorts[i].duplicatedFrom == "")
                temp->activateOutPortAsRemote<kernels::CudaORBDetectorOutMarkerType>(
                    outPorts[i].portName,
                    outPorts[i].connectingAddr,
                    outPorts[i].connectingPortNum);
              else
                temp->duplicateOutPortAsRemote<kernels::CudaORBDetectorOutMarkerType>(
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

