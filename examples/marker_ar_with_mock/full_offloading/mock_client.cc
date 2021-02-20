#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

using namespace std;

int width, height;
std::string fixedImagePath;

int main(int argc, char const *argv[])
{
  string mxre_home = getenv("MXRE_HOME");
  string config_yaml = mxre_home + "/examples/marker_ar_with_mock/full_offloading/config.yaml";

  if(mxre_home.empty()) {
    cout << "Set MXRE_HOME as a environment variable" << endl;
    return 0;
  }
  else
    cout << config_yaml << endl;


  YAML::Node config = YAML::LoadFile(config_yaml);

  width = config["width"].as<int>();
  height = config["height"].as<int>();

  fixedImagePath = config["fixed_image_path"].as<string>();

  // 1. create & run a sending pipeline
  raft::map clientPipe;

  mxre::kernels::MockCamera mockCamera(fixedImagePath, width, height);
  mockCamera.setSleepPeriodMS(16);
  mxre::kernels::RTPFrameSender rtpSender(config["codec"].as<string>(),
                                          config["server_addr"].as<string>(),
                                          config["server_video_port"].as<int>(), 800000, 10, width, height);

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::MessageSender<char> keySender(config["server_addr"].as<string>(),
                                               config["server_key_port"].as<int>(),
                                               mxre::utils::sendPrimitive<char>);

  clientPipe += mockCamera["out_frame"] >> rtpSender["in_data"];
  clientPipe += keyboard["out_keystroke"] >> keySender["in_data"];

  mxre::kernels::RTPFrameReceiver rtpReceiver(config["codec"].as<string>(),
                                              config["client_video_port"].as<int>(),
                                              width, height);
  mxre::kernels::NonDisplay nonDisplay;

  clientPipe += rtpReceiver["out_data"] >> nonDisplay["in_frame"];

  clientPipe.exe();

  return 0;
}

