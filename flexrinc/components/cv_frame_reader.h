#ifndef __FLEXR_COMPONENTS_CVFRAMEREADER__
#define __FLEXR_COMPONENTS_CVFRAMEREADER__

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "types/frame.h"

namespace flexr {
  namespace components {

    /**
     * @brief Component for live camera stream with OpenCV
     * @details
     */
    class CVFrameReader {
    private:
      cv::VideoCapture instance;
      cv::Mat intrinsic;
      cv::Mat distCoeffs;
      int width, height;

    public:
      /**
       * @brief Initialize OpenCV camera
       * @param devIdx
       *  Camera device index
       * @param width
       *  Camera frame width
       * @param height
       *  Camera frame height
       */
      CVFrameReader(int devIdx, int width, int height);


      ~CVFrameReader();


      /**
       * @brief Set camera intrinsic matrix
       * @details refer [here](https://www.mathworks.com/help/vision/ug/camera-calibration.html) for more detail of
       *  the camera calibration
       * @param inIntrinsic
       *  3x3 camera intrinsic matrix to set
       */
      void setIntrinsic(cv::Mat inIntrinsic) {intrinsic = inIntrinsic.clone();}


      /**
       * @brief Set camera distortion coefficients
       * @details refer [here](https://www.mathworks.com/help/vision/ug/camera-calibration.html) for more detail of
       *  the camera calibration
       * @param inDistCoeffs
       *  4x1 distortion coefficient matrix to set
       */
      void setDistCoeffs(cv::Mat inDistCoeffs) {distCoeffs = inDistCoeffs.clone();}


      /**
       * @brief Get camera intrinsic matrix
       * @return Camera intrinsic matrix as cv::Mat
       */
      cv::Mat getIntrinsic() {return intrinsic;}


      /**
       * @brief Get camera distortion coefficients
       * @return Camera distortion coefficient matrix as cv::Mat
       */
      cv::Mat getDistCoeffs() {return distCoeffs;}


      /**
       * @brief Read the camera frame from OpenCV VideoCapture
       * @return Read frame as flexr::types::Frame
       */
      flexr::types::Frame readFrame();
    };

  }
}

#endif

