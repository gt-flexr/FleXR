#ifndef __MXRE_CAM__
#define __MXRE_CAM__

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
    namespace input_srcs
    {
      class Camera : public raft::kernel
      {
      private:
        cv::VideoCapture cam;
        int frame_idx;
        cv::Mat intrinsic;
        cv::Mat distCoeffs;

      public:
        Camera(int dev_idx=0);
        ~Camera();
        void setIntrinsic(cv::Mat inIntrinsic) {intrinsic = inIntrinsic;}
        void setDistCoeffs(cv::Mat inDistCoeffs) {distCoeffs = inDistCoeffs;}
        cv::Mat getIntrinsic() {return intrinsic;}
        cv::Mat getDistCoeffs() {return distCoeffs;}
        virtual raft::kstatus run();
      };
    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre

#endif

