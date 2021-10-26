#pragma once

#ifdef __FLEXR_KERNEL_CV_DISPLAY__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {

    using CVDisplayMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to display frames
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * in_frame       | @ref flexr::types::Message<@ref flexr::types::Frame>
     */
    class CVDisplay : public FleXRKernel
    {
    public:

      /**
       * @brief Initialize display kernel
       * @param id
       *  Kernel ID
       */
      CVDisplay(std::string id);

      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

