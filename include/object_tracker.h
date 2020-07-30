#ifndef __MXRE_OBJ_TRACKER__
#define __MXRE_OBJ_TRACKER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"
#include "cv_types.h"

namespace mxre
{
  namespace cv_units
  {
    class ObjectTracker
    {
    private:
      cv::Ptr<cv::Feature2D> detector;
      cv::Ptr<cv::DescriptorMatcher> matcher;
      std::vector<ObjectInfo> objInfo;
      int numOfObjs;

    public:
      ObjectTracker(cv::Ptr<cv::Feature2D> _detector, cv::Ptr<cv::DescriptorMatcher> _matcher):
        detector(_detector), matcher(_matcher), numOfObjs(0) {}
      void registerObject(const cv::Mat frame, cv::Rect roiRect);
      void printRegisteredObjects();
      std::vector<ObjectInfo> getRegisteredObjects() { return objInfo; }
    };
  } // namespace cv_units
} // namespace mxre

#endif