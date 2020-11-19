#ifndef __MXRE_OBJ_DETECTOR__
#define __MXRE_OBJ_DETECTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

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

    class ORBDetector : public MXREKernel
    {
      private:
        std::vector<mxre::cv_types::ObjectInfo> objInfos;
        cv::Ptr<cv::Feature2D> detector;
        cv::Ptr<cv::DescriptorMatcher> matcher;

        double knnMatchRatio;
        int knnParam;
        double ransacThresh;
        int minInlierThresh;

      public:
        ORBDetector(std::vector<mxre::cv_types::ObjectInfo> registeredObjs);
        ~ORBDetector();
        bool logic(mxre::types::Frame *inFrame, std::vector<mxre::cv_types::ObjectInfo> *outObjInfo);
        virtual raft::kstatus run();
    };

  }   // namespace pipeline
} // namespace mxre

#endif

