#pragma once
#include <string>
#include <memory>
#include <functional>
#include <raft>
#include "raftinc/port.hpp"
#include <kernels/kernel.h>

#include <math.h>
#include <eigen3/Eigen/Dense>

#include "kimera-vio/pipeline/Pipeline.h"
#include "kernels/perceptions/kimera/kimera_vio.h"
#define NANO_SEC 1000000000.0
#include "types/kimera/types.h"

 namespace mxre
{
  namespace kernels
  {

    class KimeraVIOKernel : public MXREKernel{
		public:
      KimeraVIOKernel();
      void pose_callback(const std::shared_ptr<VIO::BackendOutput>& vio_output);
      raft::kstatus run();
      void feed_imu_cam(kimera_type::imu_cam_type *datum);
		private:
      std::chrono::high_resolution_clock::time_point start; 
      const kimera_type::imu_cam_type* datum;
      double previous_timestamp = 0.0;
      
      kimera_type::imu_cam_type* imu_cam_buffer;
      std::shared_ptr<VIO::BackendOutput> vio_output;
      VIO::VioParams kimera_pipeline_params;
      VIO::Pipeline kimera_pipeline;

      // Port *output_port
    };
  }
}