#pragma once

#ifdef __FLEXR_KERNEL_NON_DISPLAY__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using NonDisplayMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to sink frames
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * in_frame       | @ref flexr::types::Message<@ref flexr::types::Frame>
     */
    class NonDisplay : public FleXRKernel
    {
    public:
      NonDisplay();
      NonDisplay(std::string id);
      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

