#ifndef __MXRE_ILLIXR_SINK__
#define __MXRE_ILLIXR_SINK__

#include <bits/stdc++.h>
#include <zmq.h>

namespace mxre {
  namespace types {

    template<typename IN_T>
    class ILLIXRSink {
    private:
      void *ctx;
      void *sock;

    public:
      ILLIXRSink() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRSink() {
        if(sock) zmq_close(sock);
        if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REP);
        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());
      }

      int recv(IN_T *data) {
        if(sock == NULL) {
          std::cerr << "ILLIXRSink is not set." << std::endl;
          return -1;
        }
        zmq_recv(sock, data, sizeof(*data), 0);
        zmq_send(sock, "ack", 3, 0);
        return 1;
      }
    };

  }
}

#endif

