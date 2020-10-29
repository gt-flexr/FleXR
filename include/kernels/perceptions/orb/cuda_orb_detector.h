#ifdef __USE_OPENCV_CUDA__
#ifndef __MXRE_CUDA_OBJ_DETECTOR__
#define __MXRE_CUDA_OBJ_DETECTOR__

#include <bits/stdc++.h>
#include <opencv2/core/cvstd.hpp>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "utils/cv_utils.h"
#include "types/cv/types.h"
#include "types/clock_types.h"

namespace mxre
{
  namespace kernels
  {

    class CudaORBDetector : public raft::kernel
    {
      private:
        std::vector<mxre::cv_types::ObjectInfo> objInfoVec;
        cv::Ptr<cv::cuda::ORB> detector;
        cv::Ptr<cv::cuda::DescriptorMatcher> matcher;
        cv::cuda::Stream stream;
        cv::cuda::GpuMat cuFrame;
        cv::cuda::GpuMat cuKp, cuDesc;
        cv::cuda::GpuMat cuMatches;

        double knnMatchRatio;
        int knnParam;
        double ransacThresh;
        int minInlierThresh;

      public:
        CudaORBDetector(std::vector<mxre::cv_types::ObjectInfo> registeredObjs);
        ~CudaORBDetector();
        virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif
#endif
