#ifndef __FLEXR_COMPONENTS_ZMQPORT__
#define __FLEXR_COMPONENTS_ZMQPORT__

#include <zmq.h>
#include <zmq.hpp>
#include <types/types.h>

namespace flexr
{
  namespace components
  {

    /**
     * @brief Component for remote port communication based on ZMQ
     */
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


      /**
       * @brief Bind (listen) to the given port for receiving
       * @param port
       *  Port number to bind
       */
      void bind(int port) {
        int rcvhwm = 10; // in-bound high watermark
        assert(0 == zmq_setsockopt(socket, ZMQ_RCVHWM, &rcvhwm, sizeof(rcvhwm)));
        socket.bind("tcp://*:" + std::to_string(port));
      }


      /**
       * @brief Connect to the given address and port for sending
       * @param addr
       *  Address to connect
       * @param port
       *  Port number to connect
       */
      void connect(std::string addr, int port) {
        int sndhwm = 10; // out-bound high watermark
        assert(0 == zmq_setsockopt(socket, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm)));
        socket.connect("tcp://" + addr + ":" + std::to_string(port));
      }


      bool receiveMsg(bool inBlock, uint8_t* &outBuffer, uint32_t &outSize, uint32_t &outTs)
      {
        zmq::message_t msg;

        if(inBlock)
          socket.recv(msg, zmq::recv_flags::none);
        else
          socket.recv(msg, zmq::recv_flags::dontwait);

        debug_print("received Msg Size: %ld\n", msg.size());
        if(msg.size() > 0)
        {
          outSize = msg.size();
          outBuffer = new uint8_t[outSize];
          memcpy(outBuffer, msg.data(), outSize);
          types::Message<int> *temp = (types::Message<int>*)outBuffer; // ts only
          outTs = temp->ts;
          debug_print("Received TCP msg: size(%ld), ts(%d)", msg.size(), outTs);

          return true;
        }

        return false;
      }
    };
  }
}

#endif

