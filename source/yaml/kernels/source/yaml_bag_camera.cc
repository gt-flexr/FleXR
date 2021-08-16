#include <yaml/kernels/source/yaml_bag_camera.h>

namespace flexr
{
  namespace yaml
  {

    YamlBagCamera::YamlBagCamera(): YamlFleXRKernel()
    {
      framesToCache = startIndex = 0;
      bagPath = bagTopic = "";
    }


    void YamlBagCamera::parseBagCamera(const YAML::Node &node)
    {
      parseBase(node);
      parseBagCameraSpecific(node);
    }


    void YamlBagCamera::parseBagCameraSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      framesToCache     = others["frames_to_cache"].as<int>();
      startIndex        = others["start_index"].as<int>();
      bagPath           = others["bag_path"].as<std::string>();
      bagTopic          = others["bag_topic"].as<std::string>();
    }


    void YamlBagCamera::printBagCamera()
    {
      printBase();
      printBagCameraSpecific();
    }


    void YamlBagCamera::printBagCameraSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tCache frames, starting idx: " << framesToCache << ", " << startIndex << std::endl;
      std::cout << "\tBag File: " << bagPath << ", " << bagTopic << std::endl;
    }


    void* YamlBagCamera::make()
    {
      if(baseSet && specificSet)
      {
        kernels::BagCamera *temp = new kernels::BagCamera(id);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        temp->subscribeBagFile(bagPath, bagTopic);
        temp->setFramesToCache(framesToCache, startIndex);

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].connectionType == "local")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsLocal<kernels::BagCameraMsgType>(outPorts[i].portName);
            else
              temp->duplicateOutPortAsLocal<kernels::BagCameraMsgType>(outPorts[i].duplicatedFrom,
                                                                              outPorts[i].portName);
          }
          else if(outPorts[i].connectionType == "remote")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsRemote<kernels::BagCameraMsgType>(
                  outPorts[i].portName, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            else
              temp->duplicateOutPortAsRemote<kernels::BagCameraMsgType>(
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

