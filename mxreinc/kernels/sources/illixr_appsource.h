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
        sock = zmq_socket(ctx, ZMQ_PAIR);
        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());

        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());
      }

      int recvFrame(mxre::types::Frame *data) {
        mxre::types::Frame *frame = (mxre::types::Frame*)data;
        zmq_recv(sock, frame, sizeof(mxre::types::Frame), 0);
        frame->data = new unsigned char[frame->dataSize];

        zmq_recv(sock, frame->data, frame->dataSize, 0);

        return true;
      }

      void recv_cam_type(OUT_T* cam_data_) {
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSink is not set." << std::endl;
          return;
        }
        
        mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) cam_data_;
        //TODO: check Frame creation

        if (cam_data->img0!=NULL){
          cam_data->img0->release();
          cam_data->img1->release();
        }
        else{
          cam_data->img0=new types::Frame();
          cam_data->img1=new types::Frame();
        }
        
        recvFrame(cam_data->img0);
        recvFrame(cam_data->img1);

        zmq_recv(sock, &(cam_data->time), sizeof(cam_data->time), 0);
        zmq_recv(sock, &(cam_data->imu_count), sizeof(cam_data->imu_count), 0);
        zmq_recv(sock, &(cam_data->dataset_time), sizeof(cam_data->dataset_time), 0);
        
        cam_data->imu_readings = std::shared_ptr<kimera_type::imu_type[]>(new kimera_type::imu_type[cam_data->imu_count]);
        zmq_recv(sock, cam_data->imu_readings.get(), cam_data->imu_count * sizeof(kimera_type::imu_type), 0);
        zmq_send(sock, "ack", 4, 0);

        // debug_print("MXRE RECEIVED DATA FROM ILLIXR (2), DATASET TIME: %llu", cam_data->dataset_time);
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
        auto &outData(output["out_data"].template allocate<mxre::kimera_type::imu_cam_type>());
        int recvFlag = false;
        recv_cam_type(&outData);

#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        // profile_print("Exe Time ILLIXR Appsource: %lfms", getExeTime(end, start));
#endif
        output["out_data"].send();
        return raft::proceed;
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

