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

  string clientAddr     = config["client_addr"].as<string>();
  int clientMessagePort = config["client_message_port"].as<int>();

  int serverFramePort  = config["server_frame_port"].as<int>();
  string serverEncoder = config["server_encoder"].as<string>();
  string serverDecoder = config["server_decoder"].as<string>();

  if(markerPath.empty() || clientAddr.empty() || serverEncoder.empty() || serverDecoder.empty()) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  raft::map pipeline;

  flexr::kernels::RTPFrameReceiver rtpFrameReceiver("rtp_frame_receiver", serverFramePort, serverDecoder, width, height);
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");

  flexr::kernels::CudaORBDetector cudaORBDetector("cuda_orb_detector", markerPath + "/0.png");
  cudaORBDetector.setLogger("cuda_orb_detector_logger", "cuda_orb_detector.log");
  cudaORBDetector.activateInPortAsLocal<CudaORBDetectorInFrameType>("in_frame");
  cudaORBDetector.activateOutPortAsLocal<CudaORBDetectorOutMarkerType>("out_detected_markers");

  flexr::kernels::MarkerCtxExtractor markerCtxExtractor("marker_ctx_extractor", width, height);
  markerCtxExtractor.setLogger("marker_ctx_extractor_logger", "marker_ctx_extractor.log");
  markerCtxExtractor.activateInPortAsLocal<CtxExtractorInMarkerType>("in_detected_markers");
  markerCtxExtractor.activateOutPortAsRemote<CtxExtractorOutCtxType>("out_marker_contexts",
                                                                     clientAddr, clientMessagePort);

  pipeline += rtpFrameReceiver["out_frame"] >> cudaORBDetector["in_frame"];
  pipeline += cudaORBDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];

  pipeline.exe();
  return 0;
}

