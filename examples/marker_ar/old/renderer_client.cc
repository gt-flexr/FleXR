#include <flexr>

using namespace std;
using namespace flexr::kernels;

int main(int argc, char const *argv[])
{
  string flexr_home = getenv("FLEXR_HOME");
  string config_yaml = flexr_home + "/examples/marker_ar/config.yaml";
  if(flexr_home.empty()) {
    cout << "Set FLEXR_HOME as a environment variable" << endl;
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

  // Create flexr components
  raft::map pipeline;

  flexr::kernels::BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");
  bagCam.duplicateOutPortAsLocal<BagCameraMsgType>("out_frame", "out_frame2");

  flexr::kernels::ORBDetector orbDetector("orb_detector", markerPath + "/0.png");
  orbDetector.setLogger("orb_detector_logger", "orb_detector.log");
  orbDetector.activateInPortAsLocal<ORBDetectorInFrameType>("in_frame");
  orbDetector.activateOutPortAsLocal<ORBDetectorOutMarkerType>("out_detected_markers");

  flexr::kernels::MarkerCtxExtractor markerCtxExtractor("marker_ctx_extractor", width, height);
  markerCtxExtractor.setLogger("marker_ctx_extractor_logger", "marker_ctx_extractor.log");
  markerCtxExtractor.activateInPortAsLocal<CtxExtractorInMarkerType>("in_detected_markers");
  markerCtxExtractor.activateOutPortAsRemote<CtxExtractorOutCtxType>("out_marker_contexts", "RTP",
                                                                     serverAddr, serverMessagePort2);

  flexr::kernels::Keyboard keyboard("keyboard");
  keyboard.activateOutPortAsRemote<KeyboardMsgType>("out_key", "RTP", serverAddr, serverMessagePort);

  flexr::kernels::RTPFrameSender rtpFrameSender("rtp_frame_sender", serverAddr, serverFramePort, clientEncoder,
                                                width, height, width*height*4, bagFPS);
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<FrameSenderMsgType>("in_frame");

  raft::map recvPipe;
  flexr::kernels::RTPFrameReceiver rtpFrameReceiver("rtp_frame_receiver", clientFramePort, clientDecoder, width, height);
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");

  flexr::kernels::NonDisplay nonDisplay("non_display");
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

  pipeline += bagCam["out_frame2"] >> rtpFrameSender["in_frame"];
  pipeline.link(&bagCam, "out_frame", &orbDetector, "in_frame", 1);
  pipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  std::thread sendThread(flexr::kernels::runPipeline, &pipeline);
  std::thread keyThread(flexr::kernels::runSingleKernel, &keyboard);

  recvPipe += rtpFrameReceiver["out_frame"] >> nonDisplay["in_frame"];
  std::thread recvThread(flexr::kernels::runPipeline, &recvPipe);

  sendThread.join();
  keyThread.join();
  recvThread.join();

  return 0;
}

