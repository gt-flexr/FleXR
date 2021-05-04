#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <yaml-cpp//yaml.h>

using namespace std;


int main()
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

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();

  raft::map pipeline;
  mxre::kernels::BagCamera bagCam("bag_frame", bagFile, bagTopic);
  bagCam.setFramesToCache(400, 400);
  bagCam.setFPS(bagFPS);
  bagCam.duplicateOutPort<mxre::types::Message<mxre::types::Frame>>("out_frame", "out_frame2");

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(width, height);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), width, height);
  mxre::kernels::NonDisplay nonDisplay;

  // cam - obj detector
  pipeline.link(&bagCam, "out_frame", &cudaORBDetector, "in_frame", 1);

  // obj detector - obj ctx extractor
  pipeline.link(&cudaORBDetector, "out_detected_markers", &markerCtxExtractor, "in_detected_markers", 1);

  // obj ctx extractor - obj renderer
  pipeline.link(&bagCam, "out_frame2", &objRenderer, "in_frame", 1);
  pipeline.link(&markerCtxExtractor, "out_marker_contexts", &objRenderer, "in_marker_contexts", 1);
  pipeline.link(&keyboard, "out_key", &objRenderer, "in_key", 1);

  // obj renderer - test sink
  pipeline.link(&objRenderer, "out_frame", &nonDisplay, "in_frame", 1);

  pipeline.exe();
  return 0;
}

