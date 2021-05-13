#include <mxre>

using namespace std;
using namespace mxre::kernels;

int main(int argc, char const *argv[])
{
  string mxre_home = getenv("MXRE_HOME");
  string config_yaml = mxre_home + "/examples/marker_ar/config.yaml";
  if(mxre_home.empty()) {
    cout << "Set MXRE_HOME as a environment variable" << endl;
    return 0;
  }
  else cout << config_yaml << endl;

  YAML::Node config = YAML::LoadFile(config_yaml);
  string markerPath    = config["marker_path"].as<string>();
  int width            = config["width"].as<int>();
  int height           = config["height"].as<int>();
  string bagFile       = config["bag_file"].as<string>();
  string bagTopic      = config["bag_topic"].as<string>();
  int bagFPS           = config["bag_fps"].as<int>();
  if(markerPath.empty() || bagFile.empty() || bagTopic.empty() || bagFPS == 0) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  int clientFramePort  = config["client_frame_port"].as<int>();
  string clientEncoder = config["client_encoder"].as<string>();
  string clientDecoder = config["client_decoder"].as<string>();

  string serverAddr      = config["server_addr"].as<string>();
  int serverFramePort    = config["server_frame_port"].as<int>();
  int serverMessagePort  = config["server_message_port"].as<int>();
  int serverMessagePort2 = config["server_message_port2"].as<int>();

  if(markerPath.empty() || clientEncoder.empty() || clientDecoder.empty() || serverAddr.empty()) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();


  // Create mxre components
  raft::map pipeline;

  mxre::kernels::BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");
  bagCam.duplicateOutPortAsLocal<BagCameraMsgType>("out_frame", "out_frame2");

  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  orbDetector.activateInPortAsLocal<ORBDetectorInFrameType>("in_frame");
  orbDetector.activateOutPortAsLocal<ORBDetectorOutMarkerType>("out_detected_markers");
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(width, height);
  markerCtxExtractor.activateInPortAsLocal<CtxExtractorInMarkerType>("in_detected_markers");
  markerCtxExtractor.activateOutPortAsRemote<CtxExtractorOutCtxType>("out_marker_contexts",
                                                                     serverAddr, serverMessagePort2);
  mxre::kernels::Keyboard keyboard;
  keyboard.activateOutPortAsRemote<KeyboardMsgType>("out_key", serverAddr, serverMessagePort);

  mxre::kernels::RTPFrameSender rtpFrameSender(serverAddr, serverFramePort, clientEncoder,
                                               width, height, width*height*4, bagFPS);
  rtpFrameSender.activateInPortAsLocal<FrameSenderMsgType>("in_frame");

  raft::map recvPipe;
  mxre::kernels::RTPFrameReceiver rtpFrameReceiver(clientFramePort, clientDecoder, width, height);
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");
  mxre::kernels::NonDisplay nonDisplay;
  nonDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

  pipeline += bagCam["out_frame2"] >> rtpFrameSender["in_frame"];
  pipeline.link(&bagCam, "out_frame", &orbDetector, "in_frame", 1);
  pipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  std::thread sendThread(mxre::kernels::runPipeline, &pipeline);
  std::thread keyThread(mxre::kernels::runSingleKernel, &keyboard);

  recvPipe += rtpFrameReceiver["out_frame"] >> nonDisplay["in_frame"];
  std::thread recvThread(mxre::kernels::runPipeline, &recvPipe);

  sendThread.join();
  keyThread.join();
  recvThread.join();

  return 0;
}

