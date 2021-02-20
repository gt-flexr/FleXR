#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

using namespace std;

int width, height;
std::string markerPath;

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

  markerPath = config["marker_path"].as<string>();

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + to_string(height) + "/", 0, 1, orbMarkerTracker);
  orbMarkerTracker.printRegisteredObjects();

  raft::map servingPipeline;

  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(width, height);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), width, height);
  mxre::kernels::RTPFrameReceiver rtpReceiver(config["codec"].as<string>(),
                                              config["server_video_port"].as<int>(),
                                              width, height);
  rtpReceiver.duplicateOutPort<mxre::types::Frame>("out_data", "out_data2");
  mxre::kernels::MessageReceiver<char> keyReceiver(config["server_key_port"].as<int>(),
                                                   mxre::utils::recvPrimitive<char>);
  mxre::kernels::RTPFrameSender rtpSender(config["codec"].as<string>(),
                                          config["client_addr"].as<string>(),
                                          config["client_video_port"].as<int>(),
                                          800000, 10, width, height);

  servingPipeline += rtpReceiver["out_data"] >> cudaORBDetector["in_frame"];
  servingPipeline += cudaORBDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  servingPipeline += rtpReceiver["out_data2"] >> objRenderer["in_frame"];
  servingPipeline += markerCtxExtractor["out_marker_contexts"] >> objRenderer["in_marker_contexts"];
  servingPipeline += keyReceiver["out_data"] >> objRenderer["in_keystroke"];

  servingPipeline += objRenderer["out_frame"] >> rtpSender["in_data"];
  servingPipeline.exe();
  return 0;
}

