#pragma once
#include <iostream>
#include <chrono>
#include <memory>
#include <opencv2/core/mat.hpp>
#ifdef Success
#undef Success
#endif
#include <eigen3/Eigen/Dense>
#include "types/frame.h"

namespace mxre
{
  namespace kimera_type
  {
    // Data type that combines the IMU and camera data at a certain timestamp.
	// If there is only IMU data for a certain timestamp, img0 and img1 will be null
	// time is the current UNIX time where dataset_time is the time read from the csv
	typedef struct {
		std::chrono::system_clock::time_point time;
		Eigen::Vector3f angular_v;
		Eigen::Vector3f linear_a;
		unsigned long long dataset_time;
	} imu_type;

	//TODO: check if cam_type need to use Frame type
	typedef struct{
		std::chrono::system_clock::time_point time;
		cv::Mat* img0;
		cv::Mat* img1;
		unsigned long long dataset_time;
		void release(){
			delete img0;
			delete img1;
		}
	} cam_type;

	typedef struct {
		std::chrono::system_clock::time_point time;
		types::Frame* img0;
		types::Frame* img1;
		// cv::Mat* img0;
		// cv::Mat* img1;
		unsigned int imu_count;
		std::shared_ptr<imu_type[]> imu_readings;
		unsigned long dataset_time;
		// void release(){
		// 	delete img0;
		// 	delete img1;
		// }
	} imu_cam_type;

    
    // Values needed to initialize the IMU integrator
	typedef struct {
		double gyro_noise;
		double acc_noise;
		double gyro_walk;
		double acc_walk;
		Eigen::Matrix<double,3,1> n_gravity;
		double imu_integration_sigma;
		double nominal_rate;
	} imu_params;

    // IMU biases, initialization params, and slow pose needed by the IMU integrator
	typedef struct {
		double last_cam_integration_time;
		double t_offset;
		imu_params params;
		
		Eigen::Vector3d biasAcc;
		Eigen::Vector3d biasGyro;
		Eigen::Matrix<double,3,1> position;
		Eigen::Matrix<double,3,1> velocity;
		Eigen::Quaterniond quat;
	} imu_integrator_input;

    typedef struct {
		std::chrono::system_clock::time_point sensor_time; // Recorded time of sensor data ingestion
		Eigen::Vector3f position;
		Eigen::Quaternionf orientation;
	} pose_type;

	typedef struct {
		double last_cam_integration_time;
		double t_offset;

		double imu_params_gyro_noise;
		double imu_params_acc_noise;
		double imu_params_gyro_walk;
		double imu_params_acc_walk;
		double imu_params_n_gravity[3];
		double imu_params_imu_integration_sigma;
		double imu_params_nominal_rate;
		
		double biasAcc[3];
		double biasGyro[3];
		double position[3];
		double velocity[3];
		double quat[4];
		std::chrono::system_clock::time_point sensor_time; // Recorded time of sensor data ingestion
		float pose_type_position[3];
		float orientation[4];

	} kimera_output;
  }
}