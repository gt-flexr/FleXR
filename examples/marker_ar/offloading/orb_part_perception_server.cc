#include <mxre>

using namespace std;

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

  string clientAddr    = config["client_addr"].as<string>();
  int clientMessagePort  = config["client_message_port"].as<int>();

  int serverFramePort  = config["server_frame_port"].as<int>();
  string serverEncoder = config["server_encoder"].as<string>();
  string serverDecoder = config["server_decoder"].as<string>();

  if(markerPath.empty() || clientAddr.empty() || serverEncoder.empty() || serverDecoder.empty()) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();

  raft::map pipeline;
  mxre::kernels::RTPFrameReceiver rtpFrameReceiver(serverFramePort, serverDecoder, width, height);
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(width, height);
  mxre::kernels::MessageSender<std::vector<mxre::gl_types::ObjectContext>> markerCtxSender(
      clientAddr,
      clientMessagePort,
      mxre::utils::sendPrimitiveVector<std::vector<mxre::gl_types::ObjectContext>>);

  pipeline += rtpFrameReceiver["out_frame"] >> cudaORBDetector["in_frame"];
  pipeline += cudaORBDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  pipeline += markerCtxExtractor["out_marker_contexts"] >> markerCtxSender["in_data"];

  pipeline.exe();
  return 0;
}

