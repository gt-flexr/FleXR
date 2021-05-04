#include <mxre>

using namespace std;
using ObjectContextMessageType = mxre::types::Message<std::vector<mxre::gl_types::ObjectContext>>;

class TestSink: public mxre::kernels::MXREKernel {
  public:
  TestSink() {
    addInputPort<std::vector<mxre::cv_types::DetectedMarker>>("in_detected_markers");
  }

  raft::kstatus run() {
    auto &inDetectedMarkers( input["in_detected_markers"].peek<std::vector<mxre::cv_types::DetectedMarker>>() );

    cout << inDetectedMarkers.size() << endl;
    for(int i = 0; i < inDetectedMarkers.size(); i++) {
      printf("%d===== \n\t%f, %f ", inDetectedMarkers[i].index, inDetectedMarkers[i].locationIn2D.at(2).x, inDetectedMarkers[i].locationIn2D.at(2).y);
      printf("\t%f, %f, %f", inDetectedMarkers[i].defaultLocationIn3D.at(3).x,
              inDetectedMarkers[i].defaultLocationIn3D.at(3).y,
              inDetectedMarkers[i].defaultLocationIn3D.at(3).z);
    }

    recyclePort("in_detected_markers");
    return raft::proceed;
  };
};

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

  string serverAddr   = config["server_addr"].as<string>();
  int serverFramePort  = config["server_frame_port"].as<int>();
  int serverMessagePort = config["server_message_port"].as<int>();
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
  mxre::kernels::BagCamera bagCam("bag_frame", bagFile, bagTopic);
  bagCam.setFramesToCache(400, 400);
  bagCam.setFPS(bagFPS);
  bagCam.duplicateOutPort<mxre::types::Message<mxre::types::Frame>>("out_frame", "out_frame2");
  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(width, height);
  mxre::kernels::Keyboard keyboard;

  mxre::kernels::RTPFrameSender rtpFrameSender(serverAddr, serverFramePort, clientEncoder,
                                               width, height, width*height*4, bagFPS);
  mxre::kernels::MessageSender<mxre::types::Message<char>> keySender(serverAddr, serverMessagePort, mxre::utils::sendPrimitive);
  mxre::kernels::MessageSender<ObjectContextMessageType> objectCtxSender(serverAddr, serverMessagePort2,
                                                                         mxre::utils::sendPrimitiveVector);

  raft::map recvPipe;
  mxre::kernels::RTPFrameReceiver rtpFrameReceiver(clientFramePort, clientDecoder, width, height);
  mxre::kernels::NonDisplay nonDisplay;

  pipeline += bagCam["out_frame"] >> rtpFrameSender["in_frame"];
  pipeline.link(&bagCam, "out_frame2", &orbDetector, "in_frame", 1);
  pipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  pipeline += markerCtxExtractor["out_marker_contexts"] >> objectCtxSender["in_data"];
  pipeline.link(&keyboard, "out_key", &keySender, "in_data", 1);
  std::thread sendThread(mxre::kernels::runPipeline, &pipeline);

  recvPipe += rtpFrameReceiver["out_frame"] >> nonDisplay["in_frame"];
  std::thread recvThread(mxre::kernels::runPipeline, &recvPipe);

  sendThread.join();
  recvThread.join();

  return 0;
}

