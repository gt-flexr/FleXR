#ifndef __MXRE_OBJ_CTX_EXTRACTOR__
#define __MXRE_OBJ_CTX_EXTRACTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include <glm/glm.hpp>
#include "defs.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace contextualizing
    {
      class ObjectCtxExtractor : public raft::kernel
      {
      private:
        cv::Mat camIntrinsic;
        cv::Mat camDistCoeffs;

      public:
        ObjectCtxExtractor(cv::Mat intrinsic, cv::Mat distCoeffs);
        ~ObjectCtxExtractor();
        virtual raft::kstatus run();
      };
    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre

#endif
