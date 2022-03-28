#pragma once

#ifdef __FLEXR_KERNEL_ORBSLAM_POSE_ESTIMATOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/intermediate/orbslam_pose_estimator.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml OrbSlamPoseEstimator kernel
     * @see flexr::kernels::OrbSlamPoseEstimator
     */
    class YamlOrbSlamPoseEstimator: public YamlFleXRKernel
    {
      public:
        std::string vocaPath;
        std::string slamConfigPath;
        std::string sensor;
        bool viewer;

        YamlOrbSlamPoseEstimator();

        /**
         * @brief Parse OrbSlamPoseEstimator kernel info
         * @param node
         *  YAML node to parse
         */
        void parseOrbSlamPoseEstimator(const YAML::Node &node);


        /**
         * @brief Parse specifics of OrbSlamPoseEstimator kernel
         * @param node
         *  YAML node to parse
         */
        void parseOrbSlamPoseEstimatorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed OrbSlamPoseEstimator info
         */
        void printOrbSlamPoseEstimator();


        /**
         * @brief Print parsed OrbSlamPoseEstimator specifics
         */
        void printOrbSlamPoseEstimatorSpecific();


        void* make();
    };
  }
}

#endif

