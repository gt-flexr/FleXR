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
      std::vector<ObjectInfo> objInfo;
      int numOfObjs;

    public:
      ORBMarkerTracker(cv::Ptr<cv::Feature2D> _detector, cv::Ptr<cv::DescriptorMatcher> _matcher):
        detector(_detector), matcher(_matcher), numOfObjs(0) {}
      void registerObject(const cv::Mat frame, cv::Rect roiRect);
      void printRegisteredObjects();
      std::vector<ObjectInfo> getRegisteredObjects() { return objInfo; }
    };

  } // namespace cv_types
} // namespace mxre

#endif

