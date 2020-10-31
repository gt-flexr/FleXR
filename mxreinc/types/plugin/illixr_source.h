#ifndef __MXRE_ILLIXR_SOURCE__
#define __MXRE_ILLIXR_SOURCE__

#include <bits/stdc++.h>
#include <zmq.h>
#include "defs.h"

namespace mxre {
  namespace types {

    template<typename IN_T>
    class ILLIXRSource {
    private:
      void *ctx;
      void *sock;

    public:
      ILLIXRSource() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRSource(){
          if(sock) zmq_close(sock);
          if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);
        std::string connectAddr = std::string("ipc:///tmp/") + id;
        zmq_connect(sock, connectAddr.c_str());

      }

      int send(IN_T *data) {
        char ack[4];
        if(sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSource is not set." << std::endl;
          return -1;
        }
        zmq_send(sock, data, sizeof(*data), 0);
        zmq_recv(sock, ack, sizeof(ack), 0);
        debug_print("send %d \n", *data);

        return 1;
      }
    };

  }
}

#endif

