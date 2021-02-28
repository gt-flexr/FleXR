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
  mxre::kernels::MessageSender<std::vector<mxre::cv_types::DetectedMarker>> detectedMarkerSender("localhost", 49987, mxre::utils::sendDetectedMarkers);


  //mxre::pipeline::output_sinks::CVDisplay cvDisplay;

  servingPipeline += rtpReceiver["out_data"] >> cudaORBDetector["in_frame"];
  servingPipeline += cudaORBDetector["out_detected_markers"] >> detectedMarkerSender["in_data"];

  servingPipeline.exe();
  return 0;
}

