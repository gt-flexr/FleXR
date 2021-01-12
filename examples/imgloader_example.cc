#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;

int main(int argc, char const *argv[])
{
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "720p_marker", 0, 1, orbMarkerTracker);

  orbMarkerTracker.printRegisteredObjects();

  mxre::kernels::ImageLoader imageLoader("/home/jin/github/mxre/resources/video/720p/", "video_", 1, 6, WIDTH, HEIGHT);
  imageLoader.duplicateOutPort<mxre::types::Frame>("out_frame", "out_frame2");
  imageLoader.setSleepPeriodMS(50);

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::NonDisplay nonDisplay;

  raft::map pipeline;

  // cam - obj detector
  pipeline += imageLoader["out_frame"] >> orbDetector["in_frame"];

  // obj detector - obj ctx extractor
  pipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];

  // obj ctx extractor - obj renderer
  pipeline += imageLoader["out_frame2"] >> objRenderer["in_frame"];
  pipeline += markerCtxExtractor["out_marker_contexts"] >> objRenderer["in_marker_contexts"];
  pipeline += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];

  // obj renderer - test sink
  pipeline += objRenderer["out_frame"] >> nonDisplay["in_frame"];

  pipeline.exe();
  return 0;
}

