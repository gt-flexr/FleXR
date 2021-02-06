#ifdef __USE_OPENCV_CUDA__
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
    QUEUE_SIZE = 20;
  }
  else if(argc == 6) {
    WIDTH = stoi(argv[1]);
    HEIGHT = stoi(argv[2]);
    absMarkerPath = argv[3];
    absImagePath = argv[4];
    QUEUE_SIZE = stoi(argv[5]);
  }
  else {
    cout << "usage: ./EXE WIDTH HEIGHT MARKER_PATH IMG_PATH [QUEUE_SIZE]"  << endl;
    return 0;
  }

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(absMarkerPath + to_string(HEIGHT) + "/", 0, 1, orbMarkerTracker);
  orbMarkerTracker.printRegisteredObjects();

  mxre::kernels::MockCamera mockCamera(absImagePath, WIDTH, HEIGHT);
  mockCamera.duplicateOutPort<mxre::types::Frame>("out_frame", "out_frame2");
  mockCamera.setSleepPeriodMS(16);

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  //mxre::kernels::CVDisplay cvDisplay;
  mxre::kernels::NonDisplay nonDisplay;

  raft::map pipeline;
  pipeline.link(&mockCamera, "out_frame", &cudaORBDetector, "in_frame", QUEUE_SIZE);

  // obj detector - obj ctx extractor
  pipeline.link(&cudaORBDetector, "out_detected_markers", &markerCtxExtractor, "in_detected_markers", QUEUE_SIZE);

  // obj ctx extractor - obj renderer
  pipeline.link(&mockCamera, "out_frame2", &objRenderer, "in_frame", QUEUE_SIZE);
  pipeline.link(&markerCtxExtractor, "out_marker_contexts", &objRenderer, "in_marker_contexts", QUEUE_SIZE);
  pipeline.link(&keyboard, "out_keystroke", &objRenderer, "in_keystroke", QUEUE_SIZE);

  // obj renderer - test sink
  pipeline.link(&objRenderer, "out_frame", &nonDisplay, "in_frame", QUEUE_SIZE);

  pipeline.exe();
  return 0;
}

#endif

