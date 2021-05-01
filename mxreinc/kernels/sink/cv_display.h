#ifndef __MXRE_CV_DISPLAY__
#define __MXRE_CV_DISPLAY__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "kernels/kernel.h"
#include "defs.h"
#include "types/clock_types.h"
#include "types/frame.h"

namespace mxre
{
  namespace kernels
  {

    class CVDisplay : public MXREKernel
    {
    private:
      unsigned long long int frameIndex;
    public:
      CVDisplay();
      virtual raft::kstatus run() override;
      bool logic(mxre::types::Frame *inFrame);
    };

  }   // namespace kernels
} // namespace mxre

#endif

