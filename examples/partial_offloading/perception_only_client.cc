#include "kernels/offloading/message_sender.h"
#include "kernels/offloading/sending_functions.h"
#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <raftinc/rafttypes.hpp>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;

class TestSink: public mxre::kernels::MXREKernel {
  public:
  TestSink() {
    addInputPort<std::vector<mxre::cv_types::DetectedMarker>>("in_detected_markers");
  }

  raft::kstatus run() {
    auto &inDetectedMarkers( input["in_detected_markers"].peek<std::vector<mxre::cv_types::DetectedMarker>>() );

    cout << inDetectedMarkers.size() << endl;
    for(int i = 0; i < inDetectedMarkers.size(); i++) {
      printf("%d===== \n\t%f, %f ", inDetectedMarkers[i].index, inDetectedMarkers[i].locationIn2D.at(2).x, inDetectedMarkers[i].locationIn2D.at(2).y);
      printf("\t%f, %f, %f", inDetectedMarkers[i].defaultLocationIn3D.at(3).x,
              inDetectedMarkers[i].defaultLocationIn3D.at(3).y,
              inDetectedMarkers[i].defaultLocationIn3D.at(3).z);
    }

    recyclePort("in_detected_markers");
    return raft::proceed;
  };
};

int main(int argc, char const *argv[])
{
  // Set the markers
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "marker", 0, 1, orbMarkerTracker);

  // Create mxre components
  raft::map clientDag;
  mxre::kernels::ImageLoader imageLoader("/home/jin/github/mxre/resources/video/720p/", "video_", 1, 6, WIDTH, HEIGHT);
  mxre::kernels::Keyboard keyboard; // TODO MXREKernel
  mxre::kernels::RTPFrameSender rtpSender("mjpeg", "127.0.0.1", 49985, 800000, 10, WIDTH, HEIGHT);
  mxre::kernels::MessageReceiver<std::vector<mxre::gl_types::ObjectContext>> markerCtxReceiver(49987,
                                        mxre::utils::recvPrimitiveVector<std::vector<mxre::gl_types::ObjectContext>>);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::CVDisplay cvDisplay;
  //TestSink testSink;

  // Set the components
  imageLoader.setSleepPeriodMS(100);
  imageLoader.duplicateOutPort<mxre::types::Frame>("out_frame", "out_frame2");

  // Connect the components & create a dag
  clientDag += imageLoader["out_frame"] >> rtpSender["in_data"];
  //clientDag += detectedMarkerReceiver["out_data"] >> testSink["in_detected_markers"];


  clientDag += imageLoader["out_frame2"] >> objRenderer["in_frame"];
  clientDag += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];
  clientDag += markerCtxReceiver["out_data"] >> objRenderer["in_marker_contexts"];

  clientDag += objRenderer["out_frame"] >> cvDisplay["in_frame"];

  // Run the dag
  clientDag.exe();

  return 0;
}

