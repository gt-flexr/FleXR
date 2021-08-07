#ifndef __FLEXR_ARUCO_DETECTOR__
#define __FLEXR_ARUCO_DETECTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include "defs.h"
#include "types/types.h"
#include "utils/cv_utils.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

#include "kernels/kernel.h"

namespace flexr
{
  namespace kernels
  {
    using namespace flexr::types;
    using OutMarkerPoses = Message<std::vector<ObjectPose>>;


    /**
     * @brief Kernel to detect ArUco markers
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_marker_poses     | @ref flexr::types::Message<std::vector<@ref flexr::types::ObjectPose>>
    */
    class ArUcoDetector : public FleXRKernel
    {
      private:
        cv::Mat camIntrinsic, camDistCoeffs;
        cv::Ptr<cv::aruco::Dictionary> markerDict;


      public:
      /**
       * @brief Initialize kernel with ArUco contexts and frame info
       * @param dictName
       *  ArUco marker dictionary
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       */
        ArUcoDetector(cv::aruco::PREDEFINED_DICTIONARY_NAME dictName = cv::aruco::DICT_6X6_250,
                      int width = 1920, int height = 1080);


        raft::kstatus run() override;
    };

  }   // namespace pipeline
} // namespace flexr

#endif

