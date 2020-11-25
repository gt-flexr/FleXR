#ifndef __MXRE_MESSAGE_RECEIVER__
#define __MXRE_MESSAGE_RECEIVER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.hpp>
#include "defs.h"
#include "kernels/kernel.h"
#include "types/clock_types.h"

namespace mxre
{
  namespace kernels
  {
    /* Class Deifinition */
    template<typename OUT_T>
    class MessageReceiver : public mxre::kernels::MXREKernel
    {
      private:
        void (*recv)(OUT_T*, zmq::socket_t*);
        zmq::context_t ctx;
        zmq::socket_t sock;
      public:
        MessageReceiver(int port=5555, void (*recv)(OUT_T*, zmq::socket_t*)=NULL);
        ~MessageReceiver();
        virtual raft::kstatus run();
    };


    /* Constructor */
    template<typename OUT_T>
    MessageReceiver<OUT_T>::MessageReceiver(int port, void (*recv)(OUT_T*, zmq::socket_t*)): MXREKernel() {
      sock = zmq::socket_t(ctx, zmq::socket_type::rep);
      std::string bindingAddr = "tcp://*:" + std::to_string(port);
      sock.bind(bindingAddr);

      this->recv = recv;
      addOutputPort<OUT_T>("out_data");
    }


    /* Destructor */
    template<typename OUT_T>
    MessageReceiver<OUT_T>::~MessageReceiver() {
      sock.close();
      ctx.shutdown();
      ctx.close();
    }


    /* Run */
    template<typename OUT_T>
    raft::kstatus MessageReceiver<OUT_T>::run() {
#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      auto &outData( output["out_data"].template allocate<OUT_T>() );

      if(recv != NULL) recv(&outData, &sock);
      else {
        debug_print("recv function pointer is invalid.");
      }

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      output["out_data"].send();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

#endif

