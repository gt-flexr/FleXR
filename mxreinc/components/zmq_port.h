#ifndef __MXRE_COMPONENTS_ZMQPORT__
#define __MXRE_COMPONENTS_ZMQPORT__

#include <zmq.h>
#include <zmq.hpp>

namespace mxre
{
  namespace components
  {
    class ZMQPort
    {
      zmq::context_t ctx;

      public:
      zmq::socket_t socket;

      ZMQPort() {
        socket = zmq::socket_t(ctx, zmq::socket_type::pair);
      }

      ~ZMQPort() {
        socket.close();
        ctx.shutdown();
        ctx.close();
      }

      void bind(int port) {
        int rcvhwm = 10; // in-bound high watermark
        assert(0 == zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)));
        socket.bind("tcp://*:" + std::to_string(port));
      }

      void connect(std::string addr, int port) {
        int sndhwm = 10; // out-bound high watermark
        assert(0 == zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)));
        socket.connect("tcp://" + addr + ":" + std::to_string(port));
      }
    };
  }
}

#endif

