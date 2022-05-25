#ifdef __FLEXR_KERNEL_ORBSLAM_POSE_ESTIMATOR__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlOrbSlamPoseEstimator::YamlOrbSlamPoseEstimator(): YamlFleXRKernel()
    {
      vocaPath = slamConfigPath = sensor = "";
      viewer = false;
    }


    void YamlOrbSlamPoseEstimator::parseOrbSlamPoseEstimator(const YAML::Node &node)
    {
      parseBase(node);
      parseOrbSlamPoseEstimatorSpecific(node);
    }


    void YamlOrbSlamPoseEstimator::parseOrbSlamPoseEstimatorSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      vocaPath          = others["voca_path"].as<std::string>();
      slamConfigPath    = others["slam_config_path"].as<std::string>();
      sensor            = others["sensor"].as<std::string>();
      viewer            = others["viewer"].as<bool>();
    }


    void YamlOrbSlamPoseEstimator::printOrbSlamPoseEstimator()
    {
      printBase();
      printOrbSlamPoseEstimatorSpecific();
    }


    void YamlOrbSlamPoseEstimator::printOrbSlamPoseEstimatorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tVoca Path: "   << vocaPath << std::endl;
      std::cout << "\tSlam Config Path: " << slamConfigPath << std::endl;
      std::cout << "\tSensor: " << sensor << std::endl;
      std::cout << "\tViewer: " << viewer << std::endl;
    }


    void* YamlOrbSlamPoseEstimator::make()
    {
      if(baseSet && specificSet)
      {
        kernels::OrbSlamPoseEstimator *temp = new kernels::OrbSlamPoseEstimator(id, vocaPath, slamConfigPath, sensor, viewer);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::OrbSlamPoseEstimatorImgMsgType>(inPorts[i]);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::OrbSlamPoseEstimatorImgMsgType>(inPorts[i]);
            }
          }
          else if(inPorts[i].portName == "in_imus")
          {
            if(inPorts[i].connectionType == "local")
            {
              temp->activateInPortAsLocal<kernels::OrbSlamPoseEstimatorImusMsgType>(inPorts[i]);
            }
            else if(inPorts[i].connectionType == "remote")
            {
              temp->activateInPortAsRemote<kernels::OrbSlamPoseEstimatorImusMsgType>(inPorts[i]);
            }
          }
          else debug_print("invalid input port_name %s for OrbSlamPoseEstimator", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_pose")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::OrbSlamPoseEstimatorPoseMsgType>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::OrbSlamPoseEstimatorPoseMsgType>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_pose")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::OrbSlamPoseEstimatorPoseMsgType>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::OrbSlamPoseEstimatorPoseMsgType>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for OrbSlamPoseEstimator", outPorts[i].portName.c_str());
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

