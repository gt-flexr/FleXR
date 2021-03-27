#ifndef __MXRE_MOCK_CAMERA__
#define __MXRE_MOCK_CAMERA__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "defs.h"
#include "types/frame.h"
#include "types/clock_types.h"
#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {

    class MockCamera: public MXREKernel
    {
    private:
      unsigned int frameIndex;
      std::string imagePath;
      int width, height;
      cv::Mat cachedFrame;

    public:
      MockCamera(std::string path="", int width=1280, int height=720);
      ~MockCamera();
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

