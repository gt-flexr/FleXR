#ifdef __FLEXR_KERNEL_CV_CAMERA__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlCvCamera::YamlCvCamera(): YamlFleXRKernel()
    {
      devIdx = -1;
      width = height = 0;
      fileName = "";
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
      if(others["dev_index"].IsDefined()) devIdx = others["dev_index"].as<int>();
      if(others["file_name"].IsDefined()) fileName = others["file_name"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
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
      std::cout << "\tFile Name: " << fileName << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
    }


    void* YamlCvCamera::make()
    {
      if(baseSet && specificSet)
      {
        kernels::CVCamera *temp = NULL;
        if(devIdx != -1)
          temp = new kernels::CVCamera(id, devIdx, width, height, frequency);
        else if(fileName != "")
          temp = new kernels::CVCamera(id, fileName, width, height, frequency);
        temp->setLogger(loggerId, loggerFileName);
        temp->setFrequency(frequency);

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].connectionType == "local")
            temp->activateOutPortAsLocal<kernels::CVCameraMsgType>(outPorts[i]);
          else if(outPorts[i].connectionType == "remote")
            temp->activateOutPortAsRemote<kernels::CVCameraMsgType>(outPorts[i]);
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

