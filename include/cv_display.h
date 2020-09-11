#ifndef __MXRE_CV_DISPLAY__
#define __MXRE_CV_DISPLAY__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"

namespace mxre
{
  namespace pipeline
  {
    namespace output_sinks
    {
      class CVDisplay : public raft::kernel
      {
      public:
        CVDisplay();
        ~CVDisplay();
        virtual raft::kstatus run();
      };
    } // namespace output_sinks
  }   // namespace pipeline
} // namespace mxre

#endif

