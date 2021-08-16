#include <yaml/kernels/source/yaml_rtp_frame_receiver.h>

namespace flexr
{
  namespace yaml
  {

    YamlRtpFrameReceiver::YamlRtpFrameReceiver(): YamlFleXRKernel()
    {
      decoder = "";
      width = height = 0;
    }


    void YamlRtpFrameReceiver::parseRtpFrameReceiver(const YAML::Node &node)
    {
      parseBase(node);
      parseRtpFrameReceiverSpecific(node);
    }


    void YamlRtpFrameReceiver::parseRtpFrameReceiverSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      decoder           = others["decoder"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlRtpFrameReceiver::printRtpFrameReceiver()
    {
      printBase();
      printRtpFrameReceiverSpecific();
    }


    void YamlRtpFrameReceiver::printRtpFrameReceiverSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tDecoder: " << decoder << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
    }


    void* YamlRtpFrameReceiver::make()
    {
      if(baseSet && specificSet)
      {
        kernels::RTPFrameReceiver *temp = new kernels::RTPFrameReceiver(id, inPorts[0].bindingPortNum,
                                                                        decoder, width, height);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].connectionType == "local")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsLocal<kernels::FrameReceiverMsgType>(outPorts[i].portName);
            else
              temp->duplicateOutPortAsLocal<kernels::FrameReceiverMsgType>(outPorts[i].duplicatedFrom,
                                                                           outPorts[i].portName);
          }
          else if(outPorts[i].connectionType == "remote")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsRemote<kernels::FrameReceiverMsgType>(
                  outPorts[i].portName, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            else
              temp->duplicateOutPortAsRemote<kernels::FrameReceiverMsgType>(
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

