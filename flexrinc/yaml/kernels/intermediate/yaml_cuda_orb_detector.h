#ifdef __USE_OPENCV_CUDA__
#ifndef __FLEXR_YAML_CUDA_ORB_DETECTOR__
#define __FLEXR_YAML_CUDA_ORB_DETECTOR__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/intermediate/cuda_orb_detector.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml CudaORBDetector kernel
     * @see flexr::kernels::CudaORBDetector
     */
    class YamlCudaOrbDetector: public YamlFleXRKernel
    {
      public:
        std::string markerImage;

        YamlCudaOrbDetector();

        /**
         * @brief Parse CudaORBDetector kernel info
         * @param node
         *  YAML node to parse
         */
        void parseCudaOrbDetector(const YAML::Node &node);


        /**
         * @brief Parse specifics of CudaORBDetector kernel
         * @param node
         *  YAML node to parse
         */
        void parseCudaOrbDetectorSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed CudaORBDetector info
         */
        void printCudaOrbDetector();


        /**
         * @brief Print parsed CudaORBDetector specifics
         */
        void printCudaOrbDetectorSpecific();


        void* make();
    };
  }
}

#endif
#endif

