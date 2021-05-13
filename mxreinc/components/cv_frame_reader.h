#ifndef __MXRE_COMPONENTS_CVFRAMEREADER__
#define __MXRE_COMPONENTS_CVFRAMEREADER__

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "types/frame.h"

namespace mxre {
  namespace components {

    class CVFrameReader {
    private:
      cv::VideoCapture instance;
      cv::Mat intrinsic;
      cv::Mat distCoeffs;
      int width, height;

    public:
      CVFrameReader(int devIdx, int width, int height);
      ~CVFrameReader();
      void setIntrinsic(cv::Mat inIntrinsic) {intrinsic = inIntrinsic.clone();}
      void setDistCoeffs(cv::Mat inDistCoeffs) {distCoeffs = inDistCoeffs.clone();}
      cv::Mat getIntrinsic() {return intrinsic;}
      cv::Mat getDistCoeffs() {return distCoeffs;}

      mxre::types::Frame readFrame();
    };

  }
}
#endif

