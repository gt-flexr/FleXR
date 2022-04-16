#pragma once

#ifdef __FLEXR_KERNEL_RGBA2RGB_APP_SOURCE__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <flexr_shmq.h>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    class Rgba2RgbAppSource : public FleXRKernel
    {
    private:
      uint32_t seq;
      FlexrShmQueueMeta shmQueue;
      int width, height;


    public:
      Rgba2RgbAppSource(std::string id, std::string shmqName, int maxShmqElem, int width, int height, int targetFps);

      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

