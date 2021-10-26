#pragma once

#ifdef __FLEXR_KERNEL_CUDA_ORB_CAM_LOCATOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/intermediate/cuda_orb_cam_locator.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml CudaOrbCamLocator kernel
     * @see flexr::kernels::CudaOrbCamLocator
     */
    class YamlCudaOrbCamLocator: public YamlFleXRKernel
    {
      public:
        std::string markerPath;
        int width, height;

        YamlCudaOrbCamLocator();

        /**
         * @brief Parse CudaOrbCamLocator kernel info
         * @param node
         *  YAML node to parse
         */
        void parseCudaOrbCamLocator(const YAML::Node &node);


        /**
         * @brief Parse specifics of CudaOrbCamLocator kernel
         * @param node
         *  YAML node to parse
         */
        void parseCudaOrbCamLocatorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed CudaOrbCamLocator info
         */
        void printCudaOrbCamLocator();


        /**
         * @brief Print parsed CudaOrbCamLocator specifics
         */
        void printCudaOrbCamLocatorSpecific();


        void* make();
    };
  }
}

#endif

