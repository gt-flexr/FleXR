#pragma once

#ifdef __FLEXR_KERNEL_SAMPLE_MARKER_RENDERER__

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {

    using namespace flexr::types;
    using SamMarRendFrame     = Message<Frame>;
    using SamMarRendInKey     = Message<char>;
    using SamMarRendInCamPose = Message<ObjectPose>;


    /**
     * @brief Kernel to get the camera pose from a detected ArUco marker
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_cam_pose         | @ref flexr::types::Message<@ref flexr::types::ObjectPose>
    */
    class SampleMarkerRenderer : public FleXRKernel
    {
      private:
        int width, height;
        cv::Mat camIntrinsic, camDistCoeffs;
        SamMarRendFrame cachedFrame;

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
        SampleMarkerRenderer(std::string id, int width = 1920, int height = 1080);

        raft::kstatus run() override;
    };

  }   // namespace pipeline
} // namespace flexr

#endif

