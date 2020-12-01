#ifndef __MXRE_ORB_MARKER_TRACKER__
#define __MXRE_ORB_MARKER_TRACKER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"
#include "types/cv/types.h"

namespace mxre
{
  namespace cv_types
  {

    class ORBMarkerTracker
    {
    private:
      cv::Ptr<cv::Feature2D> detector;
      cv::Ptr<cv::DescriptorMatcher> matcher;
      std::vector<MarkerInfo> markerInfo;
      int numOfObjs;

    public:
      ORBMarkerTracker();
      void registerObject(const cv::Mat frame, cv::Rect roiRect);
      void printRegisteredObjects();
      std::vector<MarkerInfo> getRegisteredObjects() { return markerInfo; }
    };

  } // namespace cv_types
} // namespace mxre

#endif

