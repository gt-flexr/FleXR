#include <cstdlib>
#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include <string>

using namespace std;

int width, height, queueSize;
std::string markerPath, fixedImagePath;

int main()
{
  string mxre_home = getenv("MXRE_HOME");
  string config_yaml = mxre_home + "/examples/marker_ar_with_mock/config.yaml";

  if(mxre_home.empty()) {
    cout << "Set MXRE_HOME as a environment variable" << endl;
    return 0;
  }
  else
    cout << config_yaml << endl;


  YAML::Node config = YAML::LoadFile(config_yaml);

  width = config["width"].as<int>();
  height = config["height"].as<int>();
  queueSize = config["mock_cam_q"]["queue_size"].as<int>();

  markerPath = config["marker_path"].as<string>();
  fixedImagePath = config["fixed_image_path"].as<string>();

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + to_string(height) + "/", 0, 1, orbMarkerTracker);
  orbMarkerTracker.printRegisteredObjects();

  mxre::kernels::MockCamera mockCamera(fixedImagePath, width, height);
  mockCamera.duplicateOutPort<mxre::types::Frame>("out_frame", "out_frame2");
  mockCamera.setSleepPeriodMS(16);


  mxre::kernels::Keyboard keyboard;
  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(width, height);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), width, height);
  mxre::kernels::NonDisplay nonDisplay;

  raft::map pipeline;

  // cam - obj detector
  pipeline.link(&mockCamera, "out_frame", &orbDetector, "in_frame", queueSize);

  // obj detector - obj ctx extractor
  pipeline.link(&orbDetector, "out_detected_markers", &markerCtxExtractor, "in_detected_markers", queueSize);

  // obj ctx extractor - obj renderer
  pipeline.link(&mockCamera, "out_frame2", &objRenderer, "in_frame", queueSize);
  pipeline.link(&markerCtxExtractor, "out_marker_contexts", &objRenderer, "in_marker_contexts", queueSize);
  pipeline.link(&keyboard, "out_keystroke", &objRenderer, "in_keystroke", queueSize);

  // obj renderer - test sink
  pipeline.link(&objRenderer, "out_frame", &nonDisplay, "in_frame", queueSize);

  pipeline.exe();
  return 0;
}

