#pragma once

#ifdef __FLEXR_KERNEL_ORBSLAM_POSE_ESTIMATOR__

#include <bits/stdc++.h>
#include <GL/glew.h>
#include <ORB_SLAM3/include/System.h>
#include <ORB_SLAM3/include/ImuTypes.h>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using OrbSlamPoseEstimatorImgMsgType  = flexr::types::Message<flexr::types::Frame>;
    using OrbSlamPoseEstimatorImusMsgType = flexr::types::Message<std::vector<flexr::types::ImuPoint>>;
    using OrbSlamPoseEstimatorPoseMsgType = flexr::types::Message<flexr::types::Frame>; // matrix

    /**
     * @brief Kernel to get images from a directory
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * in_frame       | @ref flexr::types::Message< @ref flexr::types::Frame >
     * in_imus        | @ref flexr::types::Message< std::vector< @ref flexr::types::ImuPoint> >
     * out_pose       | @ref flexr::types::Message< @ref flexr::types::Frame >
     */
    class OrbSlamPoseEstimator: public FleXRKernel
    {
      private:
        ORB_SLAM3::System *SLAM;
        ORB_SLAM3::System::eSensor sensorType;
        std::string vocaPath, slamConfigPath;
        bool viewer;
        bool isInit;

      public:
      /**
       * @brief Initialize bag camera kernel
       * @param id
       *  Kernel ID
       * @param vocaPath
       *  Voca path to read
       * @param slamConfigPath
       *  Slam config path to read
       * @param sensor
       *  Sensor type
       * @param viewer
       *  turn on/off viewer
       */
        OrbSlamPoseEstimator(std::string id, std::string vocaPath, std::string slamConfigPath, std::string sensor, bool viewer);
        ~OrbSlamPoseEstimator();

        virtual raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

