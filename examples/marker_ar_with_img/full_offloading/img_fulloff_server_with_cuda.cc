#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1920
#define HEIGHT 1080

using namespace std;


int main(int argc, char const *argv[])
{
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "1080p_marker", 0, 1, orbMarkerTracker);

  raft::map servingPipeline;
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", "localhost", 49985, WIDTH, HEIGHT);
  rtpReceiver.duplicateOutPort<mxre::types::Frame>("out_data", "out_data2");
  mxre::kernels::MessageReceiver<char> keyReceiver(49986, mxre::utils::recvPrimitive<char>);
  mxre::kernels::RTPFrameSender rtpSender("mjpeg", "127.0.0.1", 49987, 800000, 10, WIDTH, HEIGHT);

  //mxre::pipeline::output_sinks::CVDisplay cvDisplay;

  servingPipeline += rtpReceiver["out_data"] >> cudaORBDetector["in_frame"];

  servingPipeline += cudaORBDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];

  servingPipeline += rtpReceiver["out_data2"] >> objRenderer["in_frame"];
  servingPipeline += markerCtxExtractor["out_marker_contexts"] >> objRenderer["in_marker_contexts"];
  servingPipeline += keyReceiver["out_data"] >> objRenderer["in_keystroke"];

  servingPipeline += objRenderer["out_frame"] >> rtpSender["in_data"];
  servingPipeline.exe();
  return 0;
}

