#ifndef __MXRE_IMAGE_LOADER__
#define __MXRE_IMAGE_LOADER__

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

    class ImageLoader: public MXREKernel
    {
    private:
      unsigned int frameIndex;
      int maxPlaceValue;
      std::string path, stemName;
      int width, height;

    public:
      ImageLoader(std::string path="", std::string stemName="", int startIndex=0, int maxPlaceValue=0,
          int width=1280, int height=720);
      ~ImageLoader();
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

