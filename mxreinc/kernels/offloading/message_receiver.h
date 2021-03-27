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
        MessageReceiver(int port=5555, void (*recv)(OUT_T*, zmq::socket_t*)=NULL, int sockType = ZMQ_REP);
        ~MessageReceiver();
        virtual raft::kstatus run();
    };


    /* Constructor */
    template<typename OUT_T>
    MessageReceiver<OUT_T>::MessageReceiver(int port, void (*recv)(OUT_T*, zmq::socket_t*), int sockType): MXREKernel() {
      sock = zmq::socket_t(ctx, sockType);
      std::string bindingAddr = "tcp://*:" + std::to_string(port);
      sock.bind(bindingAddr);

      // for pub/sub
      if(sockType == (int)zmq::socket_type::sub)
        sock.set(zmq::sockopt::subscribe, "");

      this->recv = recv;
      addOutputPort<OUT_T>("out_data");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("message_receiver", std::to_string(pid) + "/message_receiver.log");
#endif

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
      auto &outData( output["out_data"].template allocate<OUT_T>() );

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      if(recv != NULL) recv(&outData, &sock);
      else {
        debug_print("recv function pointer is invalid.");
      }

      output["out_data"].send();

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

#endif

