#ifndef __MXRE_ILLIXR_APP_SOURCE__
#define __MXRE_ILLIXR_APP_SOURCE__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/clock_types.h"
#include "types/cv/types.h"
#include "utils/cv_utils.h"
#include "kernels/kernel.h"
#include "types/kimera/types.h"

namespace mxre
{
  namespace kernels
  {

    template<typename OUT_T>
    class IllixrAppSource: public MXREKernel
    {
    protected:
      void *ctx;
      void *sock;

    public:
      IllixrAppSource() {
        addOutputPort<OUT_T>("out_data");
      }


      ~IllixrAppSource() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      void setup(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REP);
        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());

        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());
      }


      void recv_cam_type(OUT_T* cam_data_) {
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSink is not set." << std::endl;
          return;
        }
        
        mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) cam_data_;
        zmq_recv(sock, cam_data, sizeof(mxre::kimera_type::imu_cam_type), 0);

        zmq_recv(sock, cam_data->img0, sizeof(cv::Mat), 0);
        cam_data->img0->create(cam_data->img0->rows, cam_data->img0->cols, cam_data->img0->type());
        
        zmq_recv(sock, cam_data->img0->data, (cam_data->img0->total()) * (cam_data->img0->elemSize()), 0);

        zmq_recv(sock, cam_data->img1, sizeof(cv::Mat), 0);
        cam_data->img1->create(cam_data->img1->rows, cam_data->img1->cols, cam_data->img1->type());
        
        zmq_recv(sock, cam_data->img1->data, (cam_data->img1->total()) * (cam_data->img1->elemSize()), 0);
        kimera_type::imu_type* imu_readings_data = new kimera_type::imu_type[cam_data->imu_count];
        //TODO: change shared_ptr to base address of primitive array to shared_ptr<std::array<...>>
        cam_data->imu_readings = std::shared_ptr<kimera_type::imu_type>(imu_readings_data);
        zmq_recv(sock, cam_data->imu_readings.get(), cam_data->imu_count * sizeof(kimera_type::imu_type), 0);
  
        zmq_send(sock, "ack", 3, 0);
        return;
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif

        if(sock == NULL || ctx == NULL) {
          debug_print("IllixrAppSource is not set");
          return raft::stop;
        }

        auto &outData(output["out_data"].template allocate<OUT_T>());
        int recvFlag = false;
        //recvFlag = 
        recv_cam_type(&outData);
        zmq_send(sock, "ack", 3, 0);
        //TODO: recvFlag not working any more
        // if (recvFlag) {
        //   output["out_data"].send();
        // }

#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

        return raft::proceed;
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

