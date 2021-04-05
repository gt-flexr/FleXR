#ifndef __MXRE_ILLIXR_SOURCE__
#define __MXRE_ILLIXR_SOURCE__

#include <bits/stdc++.h>
#include <zmq.h>
#include <opencv/cv.hpp>
#include "defs.h"
#include "types/frame.h"
#undef Success
#include "types/kimera/types.h"


namespace mxre {
  namespace kernels {

    template<typename IN_T>
    class ILLIXRSource {
    private:
      void *ctx;
      void *sock;
      int dtype;
      char ack[4];

    public:
      ILLIXRSource() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRSource(){
          if(sock) zmq_close(sock);
          if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(std::string id, int dtype=MXRE_DTYPE_IMU_CAM) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);
        std::string connectAddr = std::string("ipc:///tmp/") + id;
        zmq_connect(sock, connectAddr.c_str());
      }

      // int send_imu_type(OUT_T* data) {
      //   if (sock == NULL || ctx == NULL) {
      //     std::cerr << "ILLIXRSource is not set." << std::endl;
      //     return -1;
      //   }

      //   std::vector<mxre::kimera_type::imu_type> *imu_data = (std::vector<mxre::kimera_type::imu_type> *) data;
      //   debug_print("Sending IMU Values");



      //   zmq_recv(sock, ack, sizeof(ack), 0);
      //   return 1;
      // }

      int send_cam_imu_type(IN_T* data) {
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSource is not set." << std::endl;
          return -1;
        }
        
        mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) data;
        zmq_send(sock, cam_data, sizeof(mxre::kimera_type::imu_cam_type), ZMQ_SNDMORE);
        debug_print("Dataset Time %llu", cam_data->dataset_time);
        
        zmq_send(sock, cam_data->img0, sizeof(cv::Mat), ZMQ_SNDMORE);
        zmq_send(sock, cam_data->img0->data(), cam_data->img0->total() * cam_data->img0->elemSize(), ZMQ_SNDMORE);

        zmq_send(sock, cam_data->img1, sizeof(cv::Mat), ZMQ_SNDMORE);
        zmq_send(sock, cam_data->img1->data(), cam_data->img1->total() * cam_data->img1->elemSize(), ZMQ_SNDMORE);

        zmq_send(sock, cam_data->imu_readings.get(), cam_data->imu_count * sizeof(mxre::kimera_type::imu_type), ZMQ_SNDMORE);
        
        zmq_recv(sock, ack, sizeof(ack), 0);
        return 1;
      }
    };

  }
}

#endif

