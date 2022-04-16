#pragma once

#ifdef __FLEXR_KERNEL_RGB2RGBA_APP_SINK__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <flexr_shmq.h>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    class Rgb2RgbaAppSink : public FleXRKernel
    {
    private:
      FlexrShmQueueMeta shmQueue;
      int width, height;
    public:
      Rgb2RgbaAppSink(std::string id, std::string shmqName, int maxShmqElem, int _width, int _height, int targetFps);
      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

