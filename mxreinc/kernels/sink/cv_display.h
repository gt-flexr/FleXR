#ifndef __FLEXR_CV_DISPLAY__
#define __FLEXR_CV_DISPLAY__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "kernels/kernel.h"
#include "defs.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "utils/msg_receiving_functions.h"

namespace mxre
{
  namespace kernels
  {
    using CVDisplayMsgType = types::Message<types::Frame>;

    class CVDisplay : public MXREKernel
    {
    public:
      enum FrameType { RGB, RGBA };
      CVDisplay(int width=1920, int height=1080, FrameType frameType=FrameType::RGB);
      raft::kstatus run() override;

    private:
      int width, height;
      FrameType frameType;
      cv::Mat inFormat;
    };

  }   // namespace kernels
} // namespace mxre

#endif

