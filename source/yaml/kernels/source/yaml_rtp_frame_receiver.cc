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

  }
}

