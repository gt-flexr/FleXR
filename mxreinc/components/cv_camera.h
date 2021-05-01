#ifndef __MXRE_COMPONENTS_CV_CAMERA__
#define __MXRE_COMPONENTS_CV_CAMERA__

#include <mxre>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

namespace mxre {
  namespace components {
    class CVCamera {
    private:
      cv::VideoCapture instance;
      uint32_t frameIndex;
      cv::Mat intrinsic;
      cv::Mat distCoeffs;
      int width, height;

    public:
      CVCamera(int devIdx, int width, int height);
      ~CVCamera();
      void setIntrinsic(cv::Mat inIntrinsic) {intrinsic = inIntrinsic.clone();}
      void setDistCoeffs(cv::Mat inDistCoeffs) {distCoeffs = inDistCoeffs.clone();}
      cv::Mat getIntrinsic() {return intrinsic;}
      cv::Mat getDistCoeffs() {return distCoeffs;}
      uint32_t getCurrentFrameIndex() {return frameIndex;}

      mxre::types::Frame readFrame();
    };
  }
}
#endif

