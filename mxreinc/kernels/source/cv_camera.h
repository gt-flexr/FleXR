#ifndef __MXRE_CV_CAMERA__
#define __MXRE_CV_CAMERA__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "utils/cv_utils.h"

namespace mxre
{
  namespace kernels
  {

    class CVCamera : public MXREKernel
    {
    private:
      cv::VideoCapture cam;
      int frameIndex;
      cv::Mat intrinsic;
      cv::Mat distCoeffs;
      int width, height;

    public:
      CVCamera(std::string id="cv_camera", int dev_idx=0, int width=1280, int height=720);
      ~CVCamera();
      void setIntrinsic(cv::Mat inIntrinsic) {intrinsic = inIntrinsic.clone();}
      void setDistCoeffs(cv::Mat inDistCoeffs) {distCoeffs = inDistCoeffs.clone();}
      cv::Mat getIntrinsic() {return intrinsic;}
      cv::Mat getDistCoeffs() {return distCoeffs;}
      virtual raft::kstatus run() override;
      bool logic(mxre::types::Frame *outFrame);
    };

  }   // namespace kernels
} // namespace mxre

#endif

