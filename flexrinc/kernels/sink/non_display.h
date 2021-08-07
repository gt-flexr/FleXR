#ifndef __FLEXR_NON_DISPLAY__
#define __FLEXR_NON_DISPLAY__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "kernels/kernel.h"
#include "defs.h"
#include "types/clock_types.h"
#include "types/frame.h"

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
      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

