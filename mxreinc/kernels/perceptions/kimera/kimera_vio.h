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
        static void pose_callback(const std::shared_ptr<VIO::BackendOutput>& vio_output, const kimera_type::imu_cam_type* datum, VIO::VioParams kimera_pipeline_params, Port* output);
        raft::kstatus run();
        void feed_imu_cam(kimera_type::imu_cam_type *datum);
        VIO::VioParams kimera_pipeline_params;
        VIO::Pipeline kimera_pipeline;
		private:
		const kimera_type::imu_cam_type* datum;
		double previous_timestamp = 0.0;
		std::shared_ptr<VIO::BackendOutput> vio_output;
    };
  }
}