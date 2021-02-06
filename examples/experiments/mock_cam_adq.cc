#include <raft>
#include <mxre>
#include <bits/stdc++.h>

using namespace std;

int WIDTH, HEIGHT, QUEUE_SIZE;
std::string absMarkerPath, absImagePath;

int main(int argc, char const *argv[])
{
  if(argc == 5) {
    WIDTH = stoi(argv[1]);
    HEIGHT = stoi(argv[2]);
    absMarkerPath = argv[3];
    absImagePath = argv[4];
  }
  else {
    cout << "usage: ./EXE WIDTH HEIGHT MARKER_PATH IMG_PATH" << endl;
    return 0;
  }

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(absMarkerPath + to_string(HEIGHT) + "/", 0, 1, orbMarkerTracker);
  orbMarkerTracker.printRegisteredObjects();

  mxre::kernels::MockCamera mockCamera(absImagePath, WIDTH, HEIGHT);
  mockCamera.duplicateOutPort<mxre::types::Frame>("out_frame", "out_frame2");
  mockCamera.setSleepPeriodMS(16);

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::NonDisplay nonDisplay;

  raft::map pipeline;

  // cam - obj detector
  pipeline += mockCamera["out_frame"] >> orbDetector["in_frame"];

  // obj detector - obj ctx extractor
  pipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];

  // obj ctx extractor - obj renderer
  pipeline += mockCamera["out_frame2"] >> objRenderer["in_frame"];
  pipeline += markerCtxExtractor["out_marker_contexts"] >> objRenderer["in_marker_contexts"];
  pipeline += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];

  // obj renderer - test sink
  pipeline += objRenderer["out_frame"] >> nonDisplay["in_frame"];

  pipeline.exe();
  return 0;
}

