#ifndef __FLEXR_CV_CAMERA__
#define __FLEXR_CV_CAMERA__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "components/cv_frame_reader.h"
#include "components/frequency_manager.h"

namespace flexr
{
  namespace kernels
  {
    using CVCameraMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to get camera frames from a camera device
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * out_frame      | @ref flexr::types::Message< @ref flexr::types::Frame>
     */
    class CVCamera : public FleXRKernel
    {
    private:
      components::CVFrameReader frameReader;
      components::FrequencyManager freqManager;
      uint32_t seq;


    public:
      /**
       * @brief Initialize camera kernel
       * @param id
       *  Kernel ID
       * @param devIdx
       *  Device index
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       * @param targetFps
       *  Target frequency to feed frames
       * @see flexr::components::CVFrameReader
       */
      CVCamera(std::string id="cv_camera", int devIdx=0, int width=1280, int height=720, int tagetFps=30);


      ~CVCamera();


      /**
       * @brief Set camera intrinsic matrix
       * @param inIntrinsic
       *  3x3 camera intrinsic matrix to set
       */
      void setIntrinsic(cv::Mat inIntrinsic);


      /**
       * @brief Set camera distortion coefficients
       * @param inDistCoeffs
       *  4x1 distortion coefficient matrix to set
       */
      void setDistCoeffs(cv::Mat inDistCoeffs);


      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

