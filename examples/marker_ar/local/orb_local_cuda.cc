#include <raft>
#include <flexr>
#include <bits/stdc++.h>
#include <yaml-cpp//yaml.h>

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

  raft::map pipeline;

  BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");
  bagCam.duplicateOutPortAsLocal<BagCameraMsgType>("out_frame", "out_frame2");

  Keyboard keyboard;
  keyboard.activateOutPortAsLocal<KeyboardMsgType>("out_key");

  CudaORBDetector cudaORBDetector("cuda_orb_detector", markerPath + "/0.png");
  cudaORBDetector.setLogger("cuda_orb_detector_logger", "cuda_orb_detector.log");
  cudaORBDetector.activateInPortAsLocal<CudaORBDetectorInFrameType>("in_frame");
  cudaORBDetector.activateOutPortAsLocal<CudaORBDetectorOutMarkerType>("out_detected_markers");

  MarkerCtxExtractor markerCtxExtractor(width, height);
  markerCtxExtractor.setLogger("marker_ctx_extractor_logger", "marker_ctx_extractor.log");
  markerCtxExtractor.activateInPortAsLocal<CtxExtractorInMarkerType>("in_detected_markers");
  markerCtxExtractor.activateOutPortAsLocal<CtxExtractorOutCtxType>("out_marker_contexts");

  ObjectRenderer objRenderer("object_renderer", width, height);
  objRenderer.setLogger("obj_renderer_logger", "obj_renderer.log");
  objRenderer.activateInPortAsLocal<ObjRendererInFrameType>("in_frame");
  objRenderer.activateInPortAsLocal<ObjRendererInKeyType>("in_key");
  objRenderer.activateInPortAsLocal<ObjRendererInCtxType>("in_marker_contexts");
  objRenderer.activateOutPortAsLocal<ObjRendererOutFrameType>("out_frame");

  NonDisplay nonDisplay("non_display");
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

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

