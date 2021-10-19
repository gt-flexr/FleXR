#ifndef __MXRE_APP_SOURCE__
#define __MXRE_APP_SOURCE__

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

namespace mxre
{
  namespace kernels
  {

    template<typename OUT_T>
    class AppSource : public MXREKernel
    {
    protected:
      void *ctx;
      void *sock;
      int dtype;

    public:
      AppSource() {
        addOutputPort<OUT_T>("out_data");
      }


      ~AppSource() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      void setup(std::string id, int dtype=MXRE_DTYPE_PRIMITIVE) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REP);
        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());

        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());
      }


      int recvPrimitive(OUT_T *data) {
        zmq_recv(sock, data, sizeof(OUT_T), 0);
        debug_print("%d \n", *data);
        return true;
      }


      int recvFrame(OUT_T *data) {
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
        
        mxre::kimera_type::cam_type *cam_data = (mxre::kimera_type::cam_type*)cam_data_;
        zmq_recv(sock, cam_data, sizeof(mxre::kimera_type::cam_type), 0);

        zmq_recv(sock, cam_data->img0, sizeof(cv::Mat),0);
        cam_data->img0->create(cam_data->img0->rows, cam_data->img0->cols, cam_data->img0->type());
        zmq_recv(sock, cam_data->img0->data,cam_data->img0->total() * cam_data->img0->elemSize(),0);
        zmq_recv(sock, cam_data->img1, sizeof(cv::Mat),0);
        cam_data->img1->create(cam_data->img1->rows, cam_data->img1->cols, cam_data->img1->type());
        zmq_recv(sock, cam_data->img1->data,cam_data->img1->total() * cam_data->img1->elemSize(),0);
  
        zmq_send(sock, "ack", 3, 0);
        return;
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif
        if(sock == NULL || ctx == NULL) {
          debug_print("AppSource is not set");
          return raft::stop;
        }

        auto &outData( output["out_data"].template allocate<OUT_T>() );
        int recvFlag = false;

        switch(dtype) {
          case MXRE_DTYPE_PRIMITIVE:
            recvFlag = recvPrimitive(&outData);
            break;
          case MXRE_DTYPE_FRAME:
            recvFlag = recvFrame(&outData);
            break;
        }
        zmq_send(sock, "ack", 3, 0);

        if(recvFlag) {
          output["out_data"].send();
          switch(dtype) {
            case MXRE_DTYPE_PRIMITIVE:
              sendPrimitiveCopy("out_data", &outData);
              break;
            case MXRE_DTYPE_FRAME:
              sendFrameCopy("out_data", &outData);
              break;
          }
        }

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

