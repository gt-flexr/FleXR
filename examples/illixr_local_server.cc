
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

void setMarker(string imgPath, mxre::cv_types::ORBMarkerTracker &orbMarkerTracker) {

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
  cv::Ptr<cv::ORB> orb = cv::ORB::create();
  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker(orb, matcher);
  setMarker("resources/illixr_markers", orbMarkerTracker);

  raft::map xrPipeline;
  mxre::kernels::CVMatAppSource appsource("source");
  mxre::kernels::Keyboard keyboard;

  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::ObjectCtxExtractor objCtxExtractor(cv::Mat(3, 3, CV_64FC1), cv::Mat(4, 1, CV_64FC1, {0, 0, 0, 0}),
      WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);

  mxre::kernels::CVMatAppSink appsink("sink");
  xrPipeline += appsource["out_data"] >>  cudaORBDetector["in_frame"];

  xrPipeline += cudaORBDetector["out_frame"] >> objCtxExtractor["in_frame"];
  xrPipeline += cudaORBDetector["out_obj_info"] >> objCtxExtractor["in_obj_info"];

  xrPipeline += objCtxExtractor["out_frame"] >> objRenderer["in_frame"];
  xrPipeline += objCtxExtractor["out_obj_context"] >> objRenderer["in_obj_context"];
  xrPipeline += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];

  xrPipeline += objRenderer["out_frame"] >> appsink["in_data"];

#ifdef __PROFILE__
  xrPipeline += appsource["frame_stamp"] >> cudaORBDetector["frame_stamp"];
  xrPipeline += cudaORBDetector["frame_stamp"] >> objCtxExtractor["frame_stamp"];
  xrPipeline += objCtxExtractor["frame_stamp"] >> objRenderer["frame_stamp"];
  xrPipeline += objRenderer["frame_stamp"] >> appsink["frame_stamp"];
#endif

  xrPipeline.exe();

  return 0;
}

