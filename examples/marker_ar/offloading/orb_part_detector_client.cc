#include <mxre>
#include <raftinc/rafttypes.hpp>

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

  int clientMessagePort  = config["client_message_port"].as<int>();
  string clientEncoder   = config["client_encoder"].as<string>();
  string clientDecoder   = config["client_decoder"].as<string>();

  string serverAddr    = config["server_addr"].as<string>();
  int serverFramePort  = config["server_frame_port"].as<int>();

  if(markerPath.empty() || clientEncoder.empty() || clientDecoder.empty() || serverAddr.empty()) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();

  raft::map pipeline;

  BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setDebugMode();
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");
  bagCam.duplicateOutPortAsLocal<BagCameraMsgType>("out_frame", "out_frame2");

  Keyboard keyboard;
  keyboard.setDebugMode();
  keyboard.activateOutPortAsLocal<KeyboardMsgType>("out_key");

  RTPFrameSender rtpFrameSender(serverAddr, serverFramePort, clientEncoder,
                                               width, height, width*height*4, bagFPS);
  rtpFrameSender.setDebugMode();
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<FrameSenderMsgType>("in_frame");

  MarkerCtxExtractor markerCtxExtractor(width, height);
  markerCtxExtractor.setLogger("marker_ctx_extractor_logger", "marker_ctx_extractor.log");
  markerCtxExtractor.activateInPortAsRemote<CtxExtractorInMarkerType>("in_detected_markers", clientMessagePort);
  markerCtxExtractor.activateOutPortAsLocal<CtxExtractorOutCtxType>("out_marker_contexts");

  ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), width, height);
  objRenderer.setDebugMode();
  objRenderer.setLogger("obj_renderer_logger", "obj_renderer.log");
  objRenderer.activateInPortAsLocal<ObjRendererInFrameType>("in_frame");
  objRenderer.activateInPortAsLocal<ObjRendererInKeyType>("in_key");
  objRenderer.activateInPortAsLocal<ObjRendererInCtxType>("in_marker_contexts");
  objRenderer.activateOutPortAsLocal<ObjRendererOutFrameType>("out_frame");

  NonDisplay nonDisplay;
  nonDisplay.setDebugMode();
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

  pipeline += bagCam["out_frame"] >> rtpFrameSender["in_frame"];

  pipeline += bagCam["out_frame2"] >> objRenderer["in_frame"];
  pipeline += markerCtxExtractor["out_marker_contexts"] >> objRenderer["in_marker_contexts"];
  pipeline.link(&keyboard, "out_key", &objRenderer, "in_key", 1);

  pipeline += objRenderer["out_frame"] >> nonDisplay["in_frame"];

  pipeline.exe();

  return 0;
}

