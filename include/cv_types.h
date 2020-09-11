#ifndef __CV_TYPES__
#define __CV_TYPES__

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

namespace mxre
{
  namespace cv_units
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

    static std::vector<cv::Point2f> convertKpsToPts(std::vector<cv::KeyPoint> keypoints)
    {
      std::vector<cv::Point2f> res;
      for (unsigned i = 0; i < keypoints.size(); i++)
      {
        res.push_back(keypoints[i].pt);
      }
      return res;
    }

    static void drawBoundingBox(cv::Mat image, std::vector<cv::Point2f> rect)
    {
      for (unsigned i = 0; i < rect.size() - 1; i++)
      {
        cv::line(image, rect[i], rect[i + 1], cv::Scalar(0, 0, 255), 2);
      }
      cv::line(image, rect[rect.size() - 1], rect[0], cv::Scalar(0, 0, 255), 2);
    }

    static void drawBoundingBox(cv::Mat image, cv::Rect rect)
    {
      cv::Point2f boxPoints[4];
      boxPoints[0].x = rect.x;            boxPoints[0].y = rect.y;
      boxPoints[1].x = rect.x+rect.width; boxPoints[1].y = rect.y;
      boxPoints[2].x = rect.x;            boxPoints[2].y = rect.y+rect.height;
      boxPoints[3].x = rect.x+rect.width; boxPoints[3].y = rect.y+rect.height;

      std::vector<cv::Point2f> vecPoints;
      vecPoints.insert(vecPoints.begin(), std::begin(boxPoints), std::end(boxPoints));
      drawBoundingBox(image, vecPoints);
    }

    static void releaseMat(cv::Mat mat) {
      mat.release();
      if(mat.data) delete [] mat.data;
    }
  } // namespace cv_units
} // namespace mxre

#endif

