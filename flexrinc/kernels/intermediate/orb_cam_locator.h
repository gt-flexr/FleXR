#ifndef __FLEXR_OBJ_DETECTOR__
#define __FLEXR_OBJ_DETECTOR__

#include <bits/stdc++.h>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"

#include "types/clock_types.h"
#include "types/frame.h"
#include "utils/cv_utils.h"

#include "kernels/kernel.h"

namespace flexr
{
  namespace kernels
  {
    using namespace flexr::types;
    using OrbCamLocatorInFrame = Message<types::Frame>;
    using OrbCamLocatorOutPose = Message<ObjectPose>;


    /**
     * @brief Kernel to locate the camera relatively by detecting an registered marker with local feature matching and ORB algorithm
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_cam_pose         | @ref flexr::types::Message<@ref flexr::types::ObjectPose>
     */
    class OrbCamLocator : public FleXRKernel
    {
      private:
        cv::Ptr<cv::Feature2D> detector;
        cv::Ptr<cv::DescriptorMatcher> matcher;

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
        OrbCamLocator(std::string id, std::string markerPath, int camWidth=1920, int camHeight=1080);

        raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

