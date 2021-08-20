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
      specificSet       = true;
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


    void* YamlCvCamera::make()
    {
      if(baseSet && specificSet)
      {
        kernels::CVCamera *temp = new kernels::CVCamera(id, devIdx, width, height, frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::CVCameraMsgType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::CVCameraMsgType>(outPorts[i].portName,
                                                                      outPorts[i].connectingAddr,
                                                                      outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::CVCameraMsgType>(outPorts[i].duplicatedFrom,
                                                                        outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::CVCameraMsgType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for CVCamera", outPorts[i].portName.c_str());
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
