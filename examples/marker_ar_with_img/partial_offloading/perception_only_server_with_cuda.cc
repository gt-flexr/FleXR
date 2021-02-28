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

  raft::map servingPipeline;
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", "localhost", 49985, WIDTH, HEIGHT);
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::MessageSender<std::vector<mxre::gl_types::ObjectContext>> markerCtxSender("localhost", 49987,
                                        mxre::utils::sendPrimitiveVector<std::vector<mxre::gl_types::ObjectContext>>);

  servingPipeline += rtpReceiver["out_data"] >> cudaORBDetector["in_frame"];
  servingPipeline += cudaORBDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];
  servingPipeline += markerCtxExtractor["out_marker_contexts"] >> markerCtxSender["in_data"];

  servingPipeline.exe();
  return 0;
}

