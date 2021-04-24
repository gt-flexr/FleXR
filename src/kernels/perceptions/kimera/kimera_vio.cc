#include "kernels/perceptions/kimera/kimera_vio.h"

 namespace mxre
{
  namespace kernels
  {

    std::string get_path() {
        std::string KIMERA_ROOT = std::string("../../src/kernels/perceptions/kimera/ILLIXR");
        return KIMERA_ROOT;
    }

    KimeraVIOKernel::KimeraVIOKernel()
    : kimera_pipeline_params(get_path())
    , kimera_pipeline(kimera_pipeline_params) {
      // addInputPort<mxre::kimera_type::cam_type>("illixr_cam_input");
	  // addInputPort<std::vector<mxre::kimera_type::imu_type>>("illixr_imu_input");
	  // TODO: check if we need std::vector in future
	  addInputPort<mxre::kimera_type::imu_cam_type>("illixr_imu_cam_input");
      addOutputPort<mxre::kimera_type::kimera_output>("kimera_pose");
	  previous_timestamp = 0;

	  kimera_pipeline.registerBackendOutputCallback(
		std::bind(
			&KimeraVIOKernel::pose_callback,
			this,
			std::placeholders::_1
		)
		);

    }

    void KimeraVIOKernel::feed_imu_cam(kimera_type::imu_cam_type *datum) {

		// Ensures that slam doesnt start before valid IMU readings come in
		if (datum == NULL) {
			assert(previous_timestamp == 0);
			return;
		}

		// This ensures that every data point is coming in chronological order If youre failing this assert, 
		// make sure that your data folder matches the name in offline_imu_cam/plugin.cc
		// debug_print("Feed_imu_cam DATASET_TIME: ")
		assert(datum->dataset_time > previous_timestamp);
		previous_timestamp = datum->dataset_time;
		imu_cam_buffer = datum;

		for (int i = 0; i < datum->imu_count; i++) {
			VIO::Vector6 imu_raw_vals;
			imu_raw_vals << datum->imu_readings.get()[i].linear_a.cast<double>(), datum->imu_readings.get()[i].angular_v.cast<double>();
			kimera_pipeline.fillSingleImuQueue(VIO::ImuMeasurement(datum->imu_readings.get()[i].dataset_time, imu_raw_vals));
		}

		cv::Mat img0{(datum->img0->useAsCVMat())};
		cv::Mat img1{(datum->img1->useAsCVMat())};

		// VIOParams
		VIO::CameraParams left_cam_info = kimera_pipeline_params.camera_params_.at(0);
		VIO::CameraParams right_cam_info = kimera_pipeline_params.camera_params_.at(1);
		const int kimera_current_frame_id = 0;//TODO: verify if this is correct
		kimera_pipeline.fillLeftFrameQueue(VIO::make_unique<VIO::Frame>(kimera_current_frame_id,
																	datum->dataset_time,
																	left_cam_info, img0));
		kimera_pipeline.fillRightFrameQueue(VIO::make_unique<VIO::Frame>(kimera_current_frame_id,
																	datum->dataset_time,
																	right_cam_info, img1));
        
		debug_print("Kimera Spinning! %llu", datum->dataset_time);
		kimera_pipeline.spin();
        return;
	}

    raft::kstatus KimeraVIOKernel::run()
    {
	  // TODO: these two lines are not done yet.
	  // kimera_type::imu_cam_type& imu_cam_data(input["illixr_cam_input"].peek<kimera_type::imu_cam_type>());
      // kimera_type::imu_type& imu_imu_data(input["illixr_imu_input"].peek<kimera_type::imu_type>());
      kimera_type::imu_cam_type& imu_cam_data(input["illixr_imu_cam_input"].peek<kimera_type::imu_cam_type>());
#ifdef __PROFILE__
      start = getNow();
#endif

	  feed_imu_cam(&imu_cam_data);
	  recyclePort("illixr_imu_cam_input");
      return raft::proceed;
    }

	kimera_type::kimera_output form_kimera_output(kimera_type::imu_integrator_input* imu_integrator_input_data, kimera_type::pose_type* pose_type_data) {
		kimera_type::kimera_output result;
		//imu_integrator_input data
		result.last_cam_integration_time= imu_integrator_input_data->last_cam_integration_time;
		result.t_offset=imu_integrator_input_data->t_offset;
		result.imu_params_gyro_noise=imu_integrator_input_data->params.gyro_noise;
		result.imu_params_acc_noise=imu_integrator_input_data->params.acc_noise;
		result.imu_params_gyro_walk=imu_integrator_input_data->params.gyro_walk;
		result.imu_params_acc_walk=imu_integrator_input_data->params.acc_walk;
		result.imu_params_n_gravity[0]=imu_integrator_input_data->params.n_gravity[0];
		result.imu_params_n_gravity[1]=imu_integrator_input_data->params.n_gravity[1];
		result.imu_params_n_gravity[2]=imu_integrator_input_data->params.n_gravity[2];
		result.imu_params_imu_integration_sigma=imu_integrator_input_data->params.imu_integration_sigma;
		result.imu_params_nominal_rate=imu_integrator_input_data->params.nominal_rate;
		result.biasAcc[0]=imu_integrator_input_data->biasAcc[0];
		result.biasAcc[1]=imu_integrator_input_data->biasAcc[1];
		result.biasAcc[2]=imu_integrator_input_data->biasAcc[2];
		result.biasGyro[0]=imu_integrator_input_data->biasGyro[0];
		result.biasGyro[1]=imu_integrator_input_data->biasGyro[1];
		result.biasGyro[2]=imu_integrator_input_data->biasGyro[2];
		result.position[0]=imu_integrator_input_data->position(0,0);
		result.position[1]=imu_integrator_input_data->position(1,0);
		result.position[2]=imu_integrator_input_data->position(2,0);
		result.velocity[0]=imu_integrator_input_data->velocity(0,0);
		result.velocity[1]=imu_integrator_input_data->velocity(1,0);
		result.velocity[2]=imu_integrator_input_data->velocity(2,0);
		result.quat[0]=imu_integrator_input_data->quat.w();
		result.quat[1]=imu_integrator_input_data->quat.x();
		result.quat[2]=imu_integrator_input_data->quat.y();
		result.quat[3]=imu_integrator_input_data->quat.z();
		//pose_type data
		result.sensor_time=pose_type_data->sensor_time;
		result.pose_type_position[0]=pose_type_data->position[0];
		result.pose_type_position[1]=pose_type_data->position[1];
		result.pose_type_position[2]=pose_type_data->position[2];
		result.orientation[0]=pose_type_data->orientation.w();
		result.orientation[1]=pose_type_data->orientation.x();
		result.orientation[2]=pose_type_data->orientation.y();
		result.orientation[3]=pose_type_data->orientation.z();
		return result;
	}


    void KimeraVIOKernel::pose_callback(const std::shared_ptr<VIO::BackendOutput>& vio_output) {
	  	//debug_print("KIMERA CALLBACK, DATASET TIME: %llu", datum->dataset_time);

		const auto& cached_state = vio_output->W_State_Blkf_;
		const auto& w_pose_blkf_trans = cached_state.pose_.translation().transpose();
		const auto& w_pose_blkf_rot = cached_state.pose_.rotation().quaternion();
		const auto& w_vel_blkf = cached_state.velocity_.transpose();
		const auto& imu_bias_gyro = cached_state.imu_bias_.gyroscope().transpose();
		const auto& imu_bias_acc = cached_state.imu_bias_.accelerometer().transpose();
		// Get the pose returned from SLAM
		Eigen::Quaternionf quat = Eigen::Quaternionf{w_pose_blkf_rot(0), w_pose_blkf_rot(1), w_pose_blkf_rot(2), w_pose_blkf_rot(3)};
		Eigen::Quaterniond doub_quat = Eigen::Quaterniond{w_pose_blkf_rot(0), w_pose_blkf_rot(1), w_pose_blkf_rot(2), w_pose_blkf_rot(3)};
		Eigen::Vector3f pos = w_pose_blkf_trans.cast<float>();

		assert(isfinite(quat.w()));
		assert(isfinite(quat.x()));
		assert(isfinite(quat.y()));
		assert(isfinite(quat.z()));
		assert(isfinite(pos[0]));
		assert(isfinite(pos[1]));
		assert(isfinite(pos[2]));

		kimera_type::pose_type* pose_type_data = new kimera_type::pose_type{
			.sensor_time = imu_cam_buffer->time,
			.position = pos,
			.orientation = quat
		};

		kimera_type::imu_integrator_input* imu_integrator_input_data = new kimera_type::imu_integrator_input{
			.last_cam_integration_time = (double(imu_cam_buffer->dataset_time) / NANO_SEC),
			.t_offset = -0.05,

			.params = {
				.gyro_noise = kimera_pipeline_params.imu_params_.gyro_noise_,
				.acc_noise = kimera_pipeline_params.imu_params_.acc_noise_,
				.gyro_walk = kimera_pipeline_params.imu_params_.gyro_walk_,
				.acc_walk = kimera_pipeline_params.imu_params_.acc_walk_,
				.n_gravity = kimera_pipeline_params.imu_params_.n_gravity_,
				.imu_integration_sigma = kimera_pipeline_params.imu_params_.imu_integration_sigma_,
				.nominal_rate = kimera_pipeline_params.imu_params_.nominal_rate_,
			},

			.biasAcc =imu_bias_acc,
			.biasGyro = imu_bias_gyro,
			.position = w_pose_blkf_trans,
			.velocity = w_vel_blkf,
			.quat = doub_quat,
		};

		kimera_type::kimera_output kimera_output_data = form_kimera_output(imu_integrator_input_data, pose_type_data);
		delete pose_type_data;
		delete imu_integrator_input_data;

        auto &outKimeraPose(output["kimera_pose"].template allocate<kimera_type::kimera_output>());
		outKimeraPose = kimera_output_data;


#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        profile_print("Exe Time Kimera: %lfms\n", getExeTime(end, start));
#endif

        output["kimera_pose"].send();
	}
  }
}