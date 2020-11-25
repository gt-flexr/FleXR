#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;


int main(int argc, char const *argv[])
{
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "marker", 0, 1, orbMarkerTracker);

  raft::map servingPipeline;
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", 49985, WIDTH, HEIGHT);
  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::MessageSender<std::vector<mxre::gl_types::ObjectContext>> markerCtxSender("localhost", 49987,
                                        mxre::utils::sendPrimitiveVector<std::vector<mxre::gl_types::ObjectContext>>);

  servingPipeline += rtpReceiver["out_data"] >> orbDetector["in_frame"];
  servingPipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  servingPipeline += markerCtxExtractor["out_marker_contexts"] >> markerCtxSender["in_data"];

  servingPipeline.exe();
  return 0;
}

