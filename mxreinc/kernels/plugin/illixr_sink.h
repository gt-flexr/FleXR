#ifndef __MXRE_ILLIXR_SINK__
#define __MXRE_ILLIXR_SINK__

#include <bits/stdc++.h>
#include <zmq.h>
#include "defs.h"
#include "types/frame.h"

namespace mxre {
  namespace kernels {

    template<typename OUT_T>
    class ILLIXRSink {
    private:
      void *ctx;
      void *sock;
      int dtype;

    public:
      ILLIXRSink() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRSink() {
        if(sock) zmq_close(sock);
        if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(std::string id, int dtype=MXRE_DTYPE_PRIMITIVE) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REP);
        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());
      }


      void recvPrimitive(OUT_T *data) {
        zmq_recv(sock, data, sizeof(OUT_T), 0);
        debug_print("%d \n", *data);
      }


      void recvFrame(void *frame) {
        mxre::types::Frame *mxreFrame = (mxre::types::Frame*)frame;
        zmq_recv(sock, mxreFrame, sizeof(mxre::types::Frame), 0);
        mxreFrame->data = new unsigned char[mxreFrame->dataSize];

        zmq_recv(sock, mxreFrame->data, mxreFrame->dataSize, 0);
      }


      int recv(OUT_T *data) {
        if(sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSink is not set." << std::endl;
          return -1;
        }

        switch(dtype) {
          case MXRE_DTYPE_PRIMITIVE:
            recvPrimitive(data);
            break;
          case MXRE_DTYPE_FRAME:
            recvFrame(data);
            break;
        }
        zmq_send(sock, "ack", 3, 0);
        return 1;
      }
    };

  }
}

#endif

