#ifndef __MXRE_ILLIXR_SOURCE__
#define __MXRE_ILLIXR_SOURCE__

#include <bits/stdc++.h>
#include <zmq.h>
#include <opencv/cv.hpp>
#include "defs.h"
#include "types/frame.h"

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


      void setup(std::string id, int dtype=MXRE_DTYPE_PRIMITIVE) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);
        std::string connectAddr = std::string("ipc:///tmp/") + id;
        zmq_connect(sock, connectAddr.c_str());
      }


      void sendPrimitive(IN_T *data) {
        zmq_send(sock, data, sizeof(IN_T), 0);
      }


      void sendFrame(void* frame) {
        mxre::types::Frame *mxreFrame = (mxre::types::Frame*) frame;
        debug_print("%d %d %d %d", mxreFrame->cols, mxreFrame->rows, mxreFrame->dataSize, mxreFrame->totalElem);

        zmq_send(sock, mxreFrame, sizeof(mxre::types::Frame), ZMQ_SNDMORE);
        zmq_send(sock, mxreFrame->data, mxreFrame->dataSize, 0);
        mxreFrame->release();
      }


      int send(IN_T *data) {
        if(sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSource is not set." << std::endl;
          return -1;
        }

        switch(dtype) {
          case MXRE_DTYPE_PRIMITIVE:
            sendPrimitive(data);
            break;
          case MXRE_DTYPE_FRAME:
            sendFrame(data);
            break;
        }
        zmq_recv(sock, ack, sizeof(ack), 0);

        return 1;
      }
    };

  }
}

#endif

