#pragma once

#ifdef __FLEXR_KERNEL_EUROC_MONO_INERTIAL_READER__


#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/source/euroc_mono_inertial_reader.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml EurocMonoInertialReader kernel
     *
     * YAML attribute | Details
     * ---------------| ----------------------------
     * img_path       | Image path
     * img_ts_path    | Image ts path
     * img_total      | Number of images
     * imu_path       | Imu path
     * imu_total      | Number of imus
     */
    class YamlEurocMonoInertialReader: public YamlFleXRKernel
    {
      private:
        std::string imgPath;
        std::string imgTsPath;
        int imgTotal;
        std::string imuPath;
        int imuTotal;

      public:
        YamlEurocMonoInertialReader();
        void parseEurocMonoInertialReader(const YAML::Node);
        void parseEurocMonoInertialReaderSpecific(const YAML::Node);
        void printEurocMonoInertialReader();
        void printEurocMonoInertialReaderSpecific();
        void* make();
    };

  }
}
#endif

