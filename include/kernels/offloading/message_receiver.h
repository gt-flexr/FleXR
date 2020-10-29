#ifndef __MXRE_MSG_RECEIVER__
#define __MXRE_MSG_RECEIVER__

#include <bits/stdc++.h>
#include <raft>

#include <zmq.h>

#include "defs.h"
#include "types/cv/types.h"

namespace mxre
{
  namespace kernels
  {

    class MessageReceiver : public raft::kernel
    {
      protected:
        void *ctx;
        void *sock;

      public:
        MessageReceiver(int port);
        ~MessageReceiver();
    };


    MessageReceiver::MessageReceiver(int port) {
      ctx = zmq_ctx_new();
      sock = zmq_socket(ctx, ZMQ_REP);

      std::string bindingAddr = "tcp://*:" + std::to_string(port);
      debug_print("Message Receiver binding %s", bindingAddr.c_str());
      zmq_bind(sock, bindingAddr.c_str());
      debug_print("Message Receiver binding %s -- DONE", bindingAddr.c_str());
    }


    MessageReceiver::~MessageReceiver() {
      zmq_close(sock);
      zmq_ctx_destroy(ctx);
    }

  }   // namespace kernels
} // namespace mxre

#endif

