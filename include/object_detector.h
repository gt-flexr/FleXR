#ifndef __MXRE_OBJ_DETECTOR__
#define __MXRE_OBJ_DETECTOR__

#include <bits/stdc++.h>
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
      class ObjectDetector : public raft::kernel
      {
        private:
          std::vector<mxre::cv_units::ObjectInfo> objInfoVec;
          cv::Ptr<cv::Feature2D> detector;
          cv::Ptr<cv::DescriptorMatcher> matcher;

          double knnMatchRatio;
          int knnParam;
          double ransacThresh;
          int minInlinerThresh;

        public:
          ObjectDetector(std::vector<mxre::cv_units::ObjectInfo> registeredObjs, cv::Ptr<cv::Feature2D> _detector,
                         cv::Ptr<cv::DescriptorMatcher> _matcher);
          ~ObjectDetector();
          virtual raft::kstatus run();
      };
    } // namespace ctx_understanding
  }   // namespace pipeline
} // namespace mxre

#endif