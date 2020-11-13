#ifndef __CV_TYPES__
#define __CV_TYPES__

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"

namespace mxre
{
  namespace cv_types
  {
    typedef struct ObjectInfo
    {
      int index;
      cv::Mat img;
      cv::Mat desc;
      std::vector<cv::KeyPoint> kps;
      std::vector<cv::Point3f> rect3D;
      std::vector<cv::Point2f> rect2D;
      std::vector<cv::Point2f> location2D;
      bool isDetected;
    } ObjectInfo;

  } // namespace cv_types
} // namespace mxre

#endif

