#ifndef __MXRE_OBJ_CTX_EXTRACTOR__
#define __MXRE_OBJ_CTX_EXTRACTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include <glm/glm.hpp>
#include "defs.h"
#include "types/cv/types.h"
#include "types/clock_types.h"

namespace mxre
{
  namespace kernels
  {

    class ObjectCtxExtractor : public raft::kernel
    {
    private:
      cv::Mat camIntrinsic;
      cv::Mat camDistCoeffs;
      int width, height;

    public:
      ObjectCtxExtractor(cv::Mat intrinsic, cv::Mat distCoeffs, int width, int height);
      ~ObjectCtxExtractor();
      virtual raft::kstatus run();
    };

  }   // namespace pipeline
} // namespace mxre

#endif
