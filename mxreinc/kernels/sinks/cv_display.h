#ifndef __MXRE_CV_DISPLAY__
#define __MXRE_CV_DISPLAY__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"
#include "types/clock_types.h"

namespace mxre
{
  namespace kernels
  {

    class CVDisplay : public raft::kernel
    {
    public:
      CVDisplay();
      ~CVDisplay();
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

