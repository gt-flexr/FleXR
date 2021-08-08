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

  string clientAddr    = config["client_addr"].as<string>();
  int clientFramePort  = config["client_frame_port"].as<int>();

  int serverFramePort    = config["server_frame_port"].as<int>();
  int serverMessagePort  = config["server_message_port"].as<int>();
  int serverMessagePort2 = config["server_message_port2"].as<int>();
  string serverEncoder   = config["server_encoder"].as<string>();
  string serverDecoder   = config["server_decoder"].as<string>();

  if(markerPath.empty() || clientAddr.empty() || serverEncoder.empty() || serverDecoder.empty()) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  flexr::cv_types::ORBMarkerTracker orbMarkerTracker;
  flexr::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<flexr::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();

  raft::map pipeline;

  flexr::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), width, height);
  objRenderer.setLogger("obj_renderer_logger", "obj_renderer.log");
  objRenderer.activateInPortAsRemote<ObjRendererInKeyType>("in_key", serverMessagePort);
  objRenderer.activateInPortAsRemote<ObjRendererInCtxType>("in_marker_contexts", serverMessagePort2);
  objRenderer.activateInPortAsLocal<ObjRendererInFrameType>("in_frame");
  objRenderer.activateOutPortAsLocal<ObjRendererOutFrameType>("out_frame");

  flexr::kernels::RTPFrameReceiver rtpFrameReceiver(serverFramePort, serverDecoder, width, height);
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");

  flexr::kernels::RTPFrameSender rtpFrameSender(clientAddr, clientFramePort, serverEncoder, width, height, width*height*4, 60);
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<FrameSenderMsgType>("in_frame");

  pipeline += rtpFrameReceiver["out_frame"] >> objRenderer["in_frame"];
  pipeline += objRenderer["out_frame"] >> rtpFrameSender["in_frame"];

  pipeline.exe();

  return 0;
}

