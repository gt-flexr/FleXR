#ifndef __MXRE_ILLIXR_SOURCE__
#define __MXRE_ILLIXR_SOURCE__

#include <bits/stdc++.h>
#include <zmq.h>
#include <opencv/cv.hpp>
#include "defs.h"

namespace mxre {
  namespace types {

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


      void setup(std::string id, int dtype=MX_DTYPE_PRIMITIVE) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);
        std::string connectAddr = std::string("ipc:///tmp/") + id;
        zmq_connect(sock, connectAddr.c_str());
      }


      void sendPrimitive(IN_T *data) {
        zmq_send(sock, data, sizeof(IN_T), 0);
      }


      void sendCVMat(void *data) {
        cv::Mat *matData = (cv::Mat*)data;
        uint matInfo[MX_MAT_ATTR_NUM];
        matInfo[MX_MAT_SIZE_IDX] = matData->total() * matData->elemSize();
        matInfo[MX_MAT_ROWS_IDX] = matData->rows;
        matInfo[MX_MAT_COLS_IDX] = matData->cols;
        matInfo[MX_MAT_TYPE_IDX] = matData->type();
        debug_print("%d %d %d %d", matInfo[0], matInfo[1], matInfo[2], matInfo[3]);

        zmq_send(sock, matInfo, sizeof(matInfo), ZMQ_SNDMORE);
        zmq_send(sock, matData->data, matInfo[MX_MAT_SIZE_IDX], 0);
      }


      int send(IN_T *data) {
        if(sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSource is not set." << std::endl;
          return -1;
        }

        switch(dtype) {
          case MX_DTYPE_PRIMITIVE:
            sendPrimitive(data);
            break;
          case MX_DTYPE_CVMAT:
            sendCVMat(data);
            break;
        }
        zmq_recv(sock, ack, sizeof(ack), 0);

        return 1;
      }
    };

  }
}

#endif

