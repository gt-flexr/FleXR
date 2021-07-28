#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <yaml-cpp//yaml.h>

using namespace std;
using namespace mxre::kernels;
using namespace mxre::types;

int main()
{
  /* Get home directory for finding config */
  string mxre_home = getenv("MXRE_HOME");
  string config_yaml = mxre_home + "/examples/marker_ar/ue4/config.yaml";
  if(mxre_home.empty()) {
    cout << "Set MXRE_HOME as a environment variable" << endl;
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

  raft::map pipeline;

  BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setDebugMode();
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(1000, 0);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");
  bagCam.duplicateOutPortAsLocal<BagCameraMsgType>("out_frame", "out_frame2");

  FrameConverter rgbaConverter("frame_converter", width, height, FrameConverter::Conversion::RGB2RGBA);
  rgbaConverter.activateInPortAsLocal<Message<Frame>>("in_frame");
  rgbaConverter.activateOutPortAsRemote<Message<Frame>>("out_frame", "127.0.0.1", appFramePort);

  Keyboard keyboard;
  keyboard.setDebugMode();
  keyboard.activateOutPortAsRemote<Message<char>>("out_key", "127.0.0.1", appKeyPort);

  //ArUcoDetector arucoDetector(cv::aruco::DICT_6X6_250, width, height);
  //arucoDetector.activateInPortAsLocal<Message<Frame>>("in_frame");
  //arucoDetector.activateOutPortAsRemote<OutMarkerPoses>("out_marker_poses", "127.0.0.1", appMarkerPort);

  ArUcoCamLocator arucoCamLocator(cv::aruco::DICT_6X6_250, width, height);
  arucoCamLocator.activateInPortAsLocal<Message<Frame>>("in_frame");
  arucoCamLocator.activateOutPortAsRemote<OutCamPose>("out_cam_pose", "127.0.0.1", appMarkerPort);



  /*
  NonDisplay nonDisplay;
  nonDisplay.setDebugMode();
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");
  */

  pipeline.link(&bagCam, "out_frame", &rgbaConverter, "in_frame", 1);
  pipeline.link(&bagCam, "out_frame2", &arucoCamLocator, "in_frame", 1);
  std::thread keyThread(mxre::kernels::runSingleKernel, &keyboard);
  pipeline.exe();

  keyThread.join();
  return 0;
}

