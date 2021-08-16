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
          if(outPorts[i].connectionType == "local")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsLocal<kernels::CVCameraMsgType>(outPorts[i].portName);
            else
              temp->duplicateOutPortAsLocal<kernels::CVCameraMsgType>(outPorts[i].duplicatedFrom,
                                                                      outPorts[i].portName);
          }
          else if(outPorts[i].connectionType == "remote")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsRemote<kernels::CVCameraMsgType>(
                  outPorts[i].portName, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            else
              temp->duplicateOutPortAsRemote<kernels::CVCameraMsgType>(
                  outPorts[i].duplicatedFrom, outPorts[i].portName,
                  outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
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
