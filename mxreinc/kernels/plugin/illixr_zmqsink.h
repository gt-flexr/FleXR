#ifndef __MXRE_ILLIXR_ZMQ_SINK__
#define __MXRE_ILLIXR_ZMQ_SINK__

#include <bits/stdc++.h>
#include <zmq.h>
#include "defs.h"
#include "types/frame.h"
#include "types/kimera/types.h"

namespace mxre {
  namespace kernels {

    template<typename OUT_T>
    class ILLIXRZMQSink {
    private:
      void *ctx;
      void *sock;
      int dtype;

    public:
      ILLIXRZMQSink() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRZMQSink() {
        if(sock) zmq_close(sock);
        if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(const char* this_machine_ip) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_PAIR);
        std::string bindingAddr = "tcp://192.17.102.20:19435";
        zmq_bind(sock, bindingAddr.c_str());
      }


      void recv_kimera_output(OUT_T* kimera_output_) {
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRZMQSink is not set." << std::endl;
          return;
        }
        
        mxre::kimera_type::kimera_output *kimera_output = (mxre::kimera_type::kimera_output*) kimera_output_;
        zmq_recv(sock, kimera_output, sizeof(mxre::kimera_type::kimera_output), 0);
        // debug_print("ILLIXR RECIEVED DATA FROM MXRE (4)");
        
        return;
      }

    };

  }
}

#endif

