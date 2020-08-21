#ifndef __MXRE_MSG_SENDER__
#define __MXRE_MSG_SENDER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>

#include "defs.h"

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      class MessageSender : public raft::kernel
      {
        protected:
          void *ctx;
          void *sock;
        public:
          MessageSender(std::string addr="localhost", int port=5555);
          ~MessageSender();
      };


      MessageSender::MessageSender(std::string addr, int port) : raft::kernel() {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);

        std::string connectingAddr = "tcp://" + addr + ":" + std::to_string(port);
        debug_print("[MessageSender] connect to %s", connectingAddr.c_str());
        zmq_connect(sock, connectingAddr.c_str());
      }

      MessageSender::~MessageSender() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif

