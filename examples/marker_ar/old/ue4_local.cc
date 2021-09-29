#include <raft>
#include <flexr>
#include <bits/stdc++.h>
#include <yaml-cpp//yaml.h>

using namespace std;
using namespace flexr::kernels;
using namespace flexr::types;

int main()
{
  /* Get home directory for finding config */
  string flexr_home = getenv("FLEXR_HOME");
  string config_yaml = flexr_home + "/examples/marker_ar/aruco/config.yaml";
  if(flexr_home.empty()) {
    cout << "Set FLEXR_HOME as a environment variable" << endl;
    return 0;
  }
  else cout << config_yaml << endl;

  /* Get config params from yaml file */
  YAML::Node config = YAML::LoadFile(config_yaml);
  int width            = config["width"].as<int>();
  int height           = config["height"].as<int>();
  string bagFile       = config["bag_file"].as<string>();
  string bagTopic      = config["bag_topic"].as<string>();
  int bagFPS           = config["bag_fps"].as<int>();
  if(bagFile.empty() || bagTopic.empty() || bagFPS == 0) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  int appKeyPort     = config["app_key_port"].as<int>();
  int appFramePort   = config["app_frame_port"].as<int>();
  int appMarkerPort     = config["app_marker_port"].as<int>();
  int flexrFramePort = config["flexr_frame_port"].as<int>();

  raft::map sendingPipeline, receivingPipeline;

  BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(1000, 0);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");
  bagCam.duplicateOutPortAsLocal<BagCameraMsgType>("out_frame", "out_frame2");

  FrameConverter rgbaConverter("frame_converter", width, height, FrameConverter::Conversion::RGB2RGBA);
  rgbaConverter.activateInPortAsLocal<Message<Frame>>("in_frame");
  rgbaConverter.activateOutPortAsRemote<Message<Frame>>("out_frame", "RTP", "127.0.0.1", appFramePort);

  Keyboard keyboard("keyboard");
  keyboard.activateOutPortAsRemote<Message<char>>("out_key", "RTP", "127.0.0.1", appKeyPort);

  ArUcoCamLocator arucoCamLocator("aruco_cam_locator", cv::aruco::DICT_6X6_250, width, height);
  arucoCamLocator.activateInPortAsLocal<flexr::kernels::ArUcoCamLocatorInFrameType>("in_frame");
  arucoCamLocator.activateOutPortAsRemote<flexr::kernels::ArUcoCamLocatorOutPoseType>("out_cam_pose", "RTP", "127.0.0.1", appMarkerPort);


  FrameConverter rgbConverter("recv_frame_converter", width, height, FrameConverter::Conversion::RGBA2RGB);
  rgbConverter.activateInPortAsRemote<Message<Frame>>("in_frame", "RTP", flexrFramePort);
  rgbConverter.activateOutPortAsLocal<Message<Frame>>("out_frame");

  CVDisplay display("cv_display");
  //display.setLogger("display_logger", "display.log");
  display.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

  sendingPipeline.link(&bagCam, "out_frame", &rgbaConverter, "in_frame", 1);
  sendingPipeline.link(&bagCam, "out_frame2", &arucoCamLocator, "in_frame", 1);

  receivingPipeline.link(&rgbConverter, "out_frame", &display, "in_frame", 1);



  std::thread sendThread(flexr::kernels::runPipeline, &sendingPipeline);
  std::thread keyThread(flexr::kernels::runSingleKernel, &keyboard);
  std::thread recvThread(flexr::kernels::runPipeline, &receivingPipeline);

  sendThread.join();
  keyThread.join();
  return 0;
}

