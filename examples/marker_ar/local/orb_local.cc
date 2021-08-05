#include <cstdlib>
#include <raft>
#include <flexr>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include <string>

using namespace std;
using namespace flexr::kernels;

int main()
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

  flexr::cv_types::ORBMarkerTracker orbMarkerTracker;
  flexr::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<flexr::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();

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

  ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  orbDetector.setDebugMode();
  orbDetector.setLogger("orb_detector_logger", "orb_detector.log");
  orbDetector.activateInPortAsLocal<ORBDetectorInFrameType>("in_frame");
  orbDetector.activateOutPortAsLocal<ORBDetectorOutMarkerType>("out_detected_markers");

  MarkerCtxExtractor markerCtxExtractor(width, height);
  markerCtxExtractor.setLogger("marker_ctx_extractor_logger", "marker_ctx_extractor.log");
  markerCtxExtractor.activateInPortAsLocal<CtxExtractorInMarkerType>("in_detected_markers");
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

  // cam - obj detector
  pipeline.link(&bagCam, "out_frame", &orbDetector, "in_frame", 1);

  // obj detector - obj ctx extractor
  pipeline.link(&orbDetector, "out_detected_markers", &markerCtxExtractor, "in_detected_markers", 1);

  // obj ctx extractor - obj renderer
  pipeline.link(&bagCam, "out_frame2", &objRenderer, "in_frame", 1);
  pipeline.link(&markerCtxExtractor, "out_marker_contexts", &objRenderer, "in_marker_contexts", 1);
  pipeline.link(&keyboard, "out_key", &objRenderer, "in_key", 1);

  // obj renderer - test sink
  pipeline.link(&objRenderer, "out_frame", &nonDisplay, "in_frame", 1);

  pipeline.exe();
  return 0;
}

