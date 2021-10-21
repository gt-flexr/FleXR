#ifndef __FLEXR_KERNEL_ARUCO_DETECTOR__
#define __FLEXR_KERNEL_ARUCO_DETECTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using namespace flexr::types;
    using ArUcoDetectorInFrameType = Message<Frame>;
    using ArUcoDetectorOutPoseType = Message<ObjectPose>;


    /**
     * @brief Kernel to detect ArUco markers
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_marker_pose      | @ref flexr::types::Message<@ref flexr::types::ObjectPose>
    */
    class ArUcoDetector : public FleXRKernel
    {
      private:
        cv::Mat camIntrinsic, camDistCoeffs;
        cv::Ptr<cv::aruco::Dictionary> markerDict;


      public:
      /**
       * @brief Initialize kernel with ArUco contexts and frame info
       * @param id
       *  Kernel ID
       * @param dictName
       *  ArUco marker dictionary
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       */
        ArUcoDetector(std::string id, cv::aruco::PREDEFINED_DICTIONARY_NAME dictName = cv::aruco::DICT_6X6_250,
                      int width = 1920, int height = 1080);


        raft::kstatus run() override;
    };

  }   // namespace pipeline
} // namespace flexr

#endif

