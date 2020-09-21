#ifndef __MXRE_CUDA_OBJ_DETECTOR__
#define __MXRE_CUDA_OBJ_DETECTOR__

#include <bits/stdc++.h>
#include <opencv2/core/cvstd.hpp>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace ctx_understanding
    {
      class CudaORBDetector : public raft::kernel
      {
        private:
          std::vector<mxre::cv_units::ObjectInfo> objInfoVec;
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
          CudaORBDetector(std::vector<mxre::cv_units::ObjectInfo> registeredObjs);
          ~CudaORBDetector();
          virtual raft::kstatus run();
      };
    } // namespace ctx_understanding
  }   // namespace pipeline
} // namespace mxre

#endif

