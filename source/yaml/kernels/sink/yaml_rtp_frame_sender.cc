#include <yaml/kernels/sink/yaml_rtp_frame_sender.h>

namespace flexr
{
  namespace yaml
  {

    YamlRtpFrameSender::YamlRtpFrameSender(): YamlFleXRKernel()
    {
      encoder = "";
      width = height = bitrate = 0;
    }


    void YamlRtpFrameSender::parseRtpFrameSender(const YAML::Node &node)
    {
      parseBase(node);
      parseRtpFrameSenderSpecific(node);
    }


    void YamlRtpFrameSender::parseRtpFrameSenderSpecific(const YAML::Node &node)
    {
      YAML::Node others = node["others"][0];
      encoder           = others["encoder"].as<std::string>();
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
      bitrate           = others["bitrate"].as<int>();
    }


    void YamlRtpFrameSender::printRtpFrameSender()
    {
      printBase();
      printRtpFrameSenderSpecific();
    }


    void YamlRtpFrameSender::printRtpFrameSenderSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tEncoder: " << encoder << std::endl;
      std::cout << "\tFrame Resolution: " << width << " x " << height << std::endl;
      std::cout << "\tEncoding Target Bitrate: " << bitrate << std::endl;
    }

  }
}

