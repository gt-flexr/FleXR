#pragma once

#ifdef __FLEXR_KERNEL_CV_CAMERA__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "flexr_core/include/core.h"

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
      CVCamera(std::string id, int devIdx, int width, int height, int tagetFps);


      ~CVCamera();


      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

