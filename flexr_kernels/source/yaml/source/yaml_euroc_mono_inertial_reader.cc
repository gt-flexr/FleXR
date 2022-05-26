#ifdef __FLEXR_KERNEL_EUROC_MONO_INERTIAL_READER__


#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlEurocMonoInertialReader::YamlEurocMonoInertialReader()
    {
      imgPath = imgTsPath = imuPath = "";
      imgTotal = imuTotal = 0;
    }


    void YamlEurocMonoInertialReader::parseEurocMonoInertialReader(const YAML::Node node)
    {
      parseBase(node);
      parseEurocMonoInertialReaderSpecific(node);
    }


    void YamlEurocMonoInertialReader::parseEurocMonoInertialReaderSpecific(const YAML::Node node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      imgPath           = others["img_path"].as<std::string>();
      imgTsPath         = others["img_ts_path"].as<std::string>();
      imgTotal          = others["img_total"].as<int>();
      imuPath           = others["imu_path"].as<std::string>();
      imuTotal          = others["imu_total"].as<int>();
    }

    void YamlEurocMonoInertialReader::printEurocMonoInertialReader()
    {
      printBase();
      printEurocMonoInertialReaderSpecific();
    }


    void YamlEurocMonoInertialReader::printEurocMonoInertialReaderSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tImg Path: " << imgPath << std::endl;
      std::cout << "\tImg Ts Path: " << imgTsPath << std::endl;
      std::cout << "\tImg Total: " << imgTotal << std::endl;
      std::cout << "\tImu Path: " << imuPath << std::endl;
      std::cout << "\tImu Total: " << imuTotal << std::endl;
    }


    void* YamlEurocMonoInertialReader::make()
    {
      if(baseSet && specificSet)
      {
        kernels::EurocMonoInertialReader *temp = new kernels::EurocMonoInertialReader(id, frequency, imgPath, imgTsPath, imgTotal, imuPath, imuTotal);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::EurocImgMsgType>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::EurocImgMsgType>(outPorts[i]);
          }
          else if(outPorts[i].portName == "out_imus")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::EurocImusMsgType>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::EurocImusMsgType>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::EurocImgMsgType>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::EurocImgMsgType>(outPorts[i]);
            }
            else if(outPorts[i].duplicatedFrom == "out_imus")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::EurocImusMsgType>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::EurocImusMsgType>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for EurocMonoInertialReader", outPorts[i].portName.c_str());
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

