#pragma once

#ifdef __FLEXR_KERNEL_EUROC_MONO_INERTIAL_READER__

#include <bits/stdc++.h>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using EurocImgMsgType  = flexr::types::Message<flexr::types::Frame>;
    using EurocImusMsgType = flexr::types::Message<std::vector<flexr::types::ImuPoint>>;


    /**
     * @brief Kernel to get images from a directory
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * out_frame      | @ref flexr::types::Message< @ref flexr::types::Frame>
     * out_imus       | @ref flexr::types::Message< std::vector< @ref flexr::types::ImuPoint> >
     */
    class EurocMonoInertialReader: public FleXRKernel
    {
      private:
        std::vector<std::string> imgFileNames;
        std::vector<double>      imgTs;
        std::vector<cv::Point3f> imuAcc, imuGyro;
        std::vector<double>      imuTs;
        int totalImgs, totalImus, firstImu;

        int seq;
        std::vector<float> imgTsTrack;

        void LoadImages(const std::string &imgPath, const std::string &imgTsPath,
                        std::vector<std::string> &imgFileNames, std::vector<double> &imgTs, int imgTotal=5000);
        void LoadImu(const std::string &imuPath, std::vector<double> &imuTs,
                     std::vector<cv::Point3f> &imuAcc, std::vector<cv::Point3f> &imuGyro, int imuTotal=5000);


      public:
      /**
       * @brief Initialize bag camera kernel
       * @param id
       *  Kernel ID
       * @param imgPath
       *  Image path to read
       * @param imgTsPath
       *  ImageTs path to read
       * @param imgTotal
       *  Total # of imgs
       * @param imuPath
       *  Imu path to read
       * @param imuTotal
       *  Total # of imus
       */
        EurocMonoInertialReader(std::string id, int fps, std::string imgPath, std::string imgTsPath, int imgTotal,
                                std::string imuPath, int imuTotal);

        virtual raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

