#ifndef __MXRE_CV_CAMERA__
#define __MXRE_CV_CAMERA__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"

namespace mxre
{
  namespace pipeline
  {
    namespace device
    {
      class CVCamera : public raft::kernel
      {
      private:
        cv::VideoCapture cam;
        int frame_idx;
        cv::Mat intrinsic;
        cv::Mat distCoeffs;
        int width, height;

      public:
        CVCamera(int dev_idx=0, int width=1280, int height=720);
        ~CVCamera();
        void setIntrinsic(cv::Mat inIntrinsic) {intrinsic = inIntrinsic;}
        void setDistCoeffs(cv::Mat inDistCoeffs) {distCoeffs = inDistCoeffs;}
        cv::Mat getIntrinsic() {return intrinsic;}
        cv::Mat getDistCoeffs() {return distCoeffs;}
        virtual raft::kstatus run();
      };
    } // namespace device
  }   // namespace pipeline
} // namespace mxre

#endif
