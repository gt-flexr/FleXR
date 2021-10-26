#pragma once

#ifdef __FLEXR_KERNEL_CUDA_ORB_CAM_LOCATOR__

#include <bits/stdc++.h>
#include <opencv2/core/cvstd.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using namespace flexr::types;
    using CudaOrbCamLocatorInFrame = Message<types::Frame>;
    using CudaOrbCamLocatorOutPose = Message<ObjectPose>;


    /**
     * @brief GPU-accelerated kernel to detect an registered marker with local feature matching and ORB algorithm
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_cam_pose         | @ref flexr::types::Message<@ref flexr::types::ObjectPose>
     */
    class CudaOrbCamLocator : public FleXRKernel
    {
      private:
        cv::Ptr<cv::cuda::ORB> detector;
        cv::Ptr<cv::cuda::DescriptorMatcher> matcher;

        cv::cuda::Stream stream;
        cv::cuda::GpuMat cuFrame;
        cv::cuda::GpuMat cuKp, cuDesc;
        cv::cuda::GpuMat cuMatches;

        std::vector<cv::KeyPoint> markerKps;
        cv::Mat                   markerDesc;
        std::vector<cv::Point3f>  markerCorner3D;
        std::vector<cv::Point2f>  markerCorner2D;

        double knnMatchRatio;
        int knnParam;
        double ransacThresh, detectionThresh;
        int minInlierThresh;

        cv::Mat camIntrinsic;
        cv::Mat camDistCoeffs;
        int width, height;

      public:
        /**
         * @brief Initialize kernel with registered marker info
         * @param id
         *  Kernel ID
         * @param markerPath
         *  Marker image file to set it as a marker
         * @param camWidth
         *  Camera frame width
         * @param camHeight
         *  Camera frame height
         */
        CudaOrbCamLocator(std::string id, std::string markerPath, int camWidth=1920, int camHeight=1080);

        raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

