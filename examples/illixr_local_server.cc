
#include "types/cv/orb_marker_tracker.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <cv.hpp>

using namespace std;

#define WIDTH 752
#define HEIGHT 480

void setMarker(std::string imgPath, mxre::cv_types::ORBMarkerTracker &orbMarkerTracker) {
  cv::Mat image;

  string imgDir = mxre::utils::PathFinder::find(imgPath);
  cout << imgDir << endl;
  vector<cv::String> imageName;
  cv::glob(imgDir + "/2.png", imageName, false);

  for(size_t i = 0; i <  imageName.size(); i++) {
    image = cv::imread(imageName[i]);
    cv::imshow("setMarker", image);

    cv::Rect roiRect = cv::selectROI("setMarker", image);
    if(roiRect.width < 100 || roiRect.height < 100) {
      std::cout << "Too small, select again" << std::endl;
      i--;
    }
    else {
      orbMarkerTracker.registerObject(image, roiRect);
    }
  }
  orbMarkerTracker.printRegisteredObjects();
}


int main(int argc, char const *argv[])
{
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  setMarker("resources/illixr_markers", orbMarkerTracker);

  raft::map xrPipeline;
  mxre::kernels::AppSource<mxre::types::Frame> appsource;
  mxre::kernels::Keyboard keyboard;

  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);

  mxre::kernels::AppSink<mxre::types::Frame> appsink;

  appsource.setup("source");
  appsource.duplicateOutPort<mxre::types::Frame>("out_data", "out_data2");
  appsink.setup("sink");

  xrPipeline += appsource["out_data"] >>  cudaORBDetector["in_frame"];
  xrPipeline += cudaORBDetector["out_obj_info"] >> markerCtxExtractor["in_obj_info"];

  xrPipeline += appsource["out_data2"] >> objRenderer["in_frame"];
  xrPipeline += markerCtxExtractor["out_obj_context"] >> objRenderer["in_obj_context"];
  xrPipeline += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];

  xrPipeline += objRenderer["out_frame"] >> appsink["in_data"];

  xrPipeline.exe();

  return 0;
}

