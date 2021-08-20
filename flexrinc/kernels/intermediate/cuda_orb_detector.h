#ifdef __USE_OPENCV_CUDA__
#ifndef __FLEXR_CUDA_OBJ_DETECTOR__
#define __FLEXR_CUDA_OBJ_DETECTOR__

#include <bits/stdc++.h>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"

#include "utils/cv_utils.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

#include "components/orb_marker_tracker.h"

#include "kernels/kernel.h"


namespace flexr
{
  namespace kernels
  {
    using CudaORBDetectorInFrameType = types::Message<types::Frame>;
    using CudaORBDetectorOutMarkerType = types::Message<std::vector<cv_types::DetectedMarker>>;


    /**
     * @brief GPU-accelerated kernel to detect an registered marker with local feature matching and ORB algorithm
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_detected_markers | @ref flexr::types::Message<std::vector<@ref flexr::cv_types::DetectedMarker>>
     */
    class CudaORBDetector : public FleXRKernel
    {
      private:
        components::OrbMarkerTracker orbMarkerTracker;
        std::vector<flexr::cv_types::MarkerInfo> registeredMarkers;
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
        int numKps;


      public:
        /**
         * @brief Initialize kernel with registered marker info
         * @param id
         *  Kernel ID
         * @param markerImage
         *  Marker image file to set it as a marker
         */
        CudaORBDetector(std::string id, std::string markerImage);


        raft::kstatus run() override;


        bool logic(CudaORBDetectorInFrameType   *inFrame,
                   CudaORBDetectorOutMarkerType *outDetectedMarkers);
    };

  }   // namespace kernels
} // namespace flexr

#endif
#endif

