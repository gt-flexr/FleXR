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

    typedef struct MarkerInfo
    {
      int index;
      cv::Mat img;
      cv::Mat desc;
      std::vector<cv::KeyPoint> kps;
      std::vector<cv::Point3f> defaultLocationIn3D;
      std::vector<cv::Point2f> defaultLocationIn2D;
    } MarkerInfo;


    typedef struct DetectedMarker {
      int index;
      std::vector<cv::Point3f> defaultLocationIn3D;
      std::vector<cv::Point2f> locationIn2D;
    } DetectedMarker;


    typedef struct Point3fForCommunication {
      float x, y, z;
    } Point3fForCommunication;


    typedef struct Point2fForCommunication {
      float x, y;
    } Point2fForCommunication;

  } // namespace cv_types
} // namespace mxre

#endif

