#ifndef __CV_UTILS__
#define __CV_UTILS__

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/cv/orb_marker_tracker.h"

namespace flexr
{
  namespace cv_utils
  {

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


    static std::string type2str(int type) {
      std::string r;

      uchar depth = type & CV_MAT_DEPTH_MASK;
      uchar chans = 0 + (type >> CV_CN_SHIFT);

      switch ( depth ) {
        case CV_8U:  r = "8U"; break;
        case CV_8S:  r = "8S"; break;
        case CV_16U: r = "16U"; break;
        case CV_16S: r = "16S"; break;
        case CV_32S: r = "32S"; break;
        case CV_32F: r = "32F"; break;
        case CV_64F: r = "64F"; break;
        default:     r = "User"; break;
      }

      r += "C";
      r += (chans+'-1');

      return r;
    }


    static void setMarkerFromImages(std::string path, std::string stemName, int startIndex, int maxPlaceValue,
        flexr::cv_types::ORBMarkerTracker &orbMarkerTracker) {
      cv::Mat image;

      while(1) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(maxPlaceValue);
        ss << startIndex++;
        std::string imagePath = path + stemName + ss.str() + ".png";
        image = cv::imread(imagePath);
        if(image.empty()) {
          break;
        }

        cv::Rect roiRect(0, 0, image.cols, image.rows);
        orbMarkerTracker.registerObject(image, roiRect);
      }
      orbMarkerTracker.printRegisteredObjects();
    }


    static void setMarkerFromImages(std::string path, int startIndex, int maxPlaceValue,
                                    flexr::cv_types::ORBMarkerTracker &orbMarkerTracker)
    {
      cv::Mat image;

      while(1) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(maxPlaceValue);
        ss << startIndex++;
        std::string imagePath = path + ss.str() + ".png";
        image = cv::imread(imagePath);
        if(image.empty()) {
          break;
        }

        cv::Rect roiRect(0, 0, image.cols, image.rows);
        orbMarkerTracker.registerObject(image, roiRect);
      }
      orbMarkerTracker.printRegisteredObjects();
    }
  } // namespace cv_utils
} // namespace flexr

#endif

