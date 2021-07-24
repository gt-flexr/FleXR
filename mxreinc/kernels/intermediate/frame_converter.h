#ifndef __FLEXR_FRAME_CONVERTER__
#define __FLEXR_FRAME_CONVERTER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>

#include "defs.h"
#include "utils/cv_utils.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {
    using namespace mxre::types;

    class FrameConverter : public MXREKernel
    {
      public:
        enum Conversion { RGB2RGBA };
        FrameConverter(std::string id, int width, int height, Conversion conv);
        raft::kstatus run() override;

      private:
        int width, height;
        Conversion conv;
    };

  }   // namespace kernels
} // namespace mxre

#endif

