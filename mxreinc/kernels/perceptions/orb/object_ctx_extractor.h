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
#include "types/gl/types.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {

    class ObjectCtxExtractor : public MXREKernel
    {
    private:
      cv::Mat camIntrinsic;
      cv::Mat camDistCoeffs;
      int width, height;

    public:
      ObjectCtxExtractor(cv::Mat intrinsic, cv::Mat distCoeffs, int width, int height);
      virtual raft::kstatus run();
      bool logic(std::vector<mxre::cv_types::ObjectInfo> *inObjInfo,
          std::vector<mxre::gl_types::ObjectContext> *outObjContext);
    };

  }   // namespace pipeline
} // namespace mxre

#endif

