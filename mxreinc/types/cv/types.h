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


    class Mat {
      public:
      bool isExt;

      cv::Mat cvMat;
      unsigned char* data;
      size_t total, elemSize;
      int rows, cols, type;

      ~Mat(){
        release(); // ??
      }

      Mat(){}

      Mat(cv::Mat inMat){
        isExt = false;
        cvMat = inMat.clone();
        setMatInfo();
      }

      Mat(int rows, int cols, int type) {
        isExt = false;
        cvMat = cv::Mat(rows, cols, type);
        setMatInfo();
      }

      Mat(int rows, int cols, int type, void *data) {
        isExt = true;
        cvMat = cv::Mat(rows, cols, type, data);
        setMatInfo();
      }


      Mat(const Mat &ref) {
        isExt = false; // ??
        cvMat = ref.cvMat.clone();
        setMatInfo();
      }

      Mat& operator=(const Mat& ref) {
        release();
        isExt = false; // false, because of cloning
        cvMat = ref.cvMat.clone();
        setMatInfo();
        return *this;
      }

      void setMatInfo() {
        total = cvMat.total();
        elemSize = cvMat.elemSize();
        type = cvMat.type();
        rows = cvMat.rows;
        cols = cvMat.cols;
        data = cvMat.data;
      }

      void release() {
        if(!cvMat.empty()) {
          cvMat.release();
          if(data && isExt) {
            delete [] data;
          }
        }
      }
    };

  } // namespace cv_types
} // namespace mxre

#endif

