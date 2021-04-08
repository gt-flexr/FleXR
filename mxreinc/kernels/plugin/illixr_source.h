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
        sock = zmq_socket(ctx, ZMQ_PAIR);
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

      void sendFrame(mxre::types::Frame *data) {
        mxre::types::Frame *frame = (mxre::types::Frame*)data;
        //cv::Mat *mat = (cv::Mat*)data;
        //mxre::types::Frame frame;
        //frame.setFrameAttribFromCVMat(*mat);
        debug_print("%d %d %d %d", frame->cols, frame->rows, frame->dataSize, frame->totalElem);

        zmq_send(sock, frame, sizeof(mxre::types::Frame), 0);
        zmq_send(sock, frame->data, frame->dataSize, 0);
        frame->release();
      }

      int send_cam_imu_type(IN_T* data) {
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSource is not set." << std::endl;
          return -1;
        }
        
        mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) data;
        sendFrame(cam_data->img0);
        sendFrame(cam_data->img1);

        zmq_send(sock, &(cam_data->time), sizeof(cam_data->time), 0);
        zmq_send(sock, &(cam_data->imu_count), sizeof(cam_data->imu_count), 0);
        zmq_send(sock, &(cam_data->dataset_time) , sizeof(cam_data->dataset_time), 0);
        zmq_send(sock, cam_data->imu_readings.get(), cam_data->imu_count * sizeof(mxre::kimera_type::imu_type), 0);
        zmq_recv(sock, ack, sizeof(ack), 0);

        // debug_print("ILLIXR SENDING DATA FROM MXRE (1), DATASET TIME: %llu", cam_data->dataset_time);
        return 1;
      }
    };

  }
}

#endif

