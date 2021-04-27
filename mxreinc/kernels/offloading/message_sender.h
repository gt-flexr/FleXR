#ifndef __MXRE_MESSAGE_SENDER__
#define __MXRE_MESSAGE_SENDER__

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
    template<typename IN_T>
    class MessageSender : public mxre::kernels::MXREKernel
    {
      private:
        void (*send)(IN_T*, zmq::socket_t*);
        zmq::context_t ctx;
        zmq::socket_t sock;
      public:
        MessageSender(std::string addr="localhost", int port=5555, void (*send)(IN_T*, zmq::socket_t*)=NULL,
                      int sockType=ZMQ_PAIR);
        ~MessageSender();
        virtual raft::kstatus run();
    };


    /* Constructor */
    template<typename IN_T>
    MessageSender<IN_T>::MessageSender(std::string addr, int port, void (*send)(IN_T*, zmq::socket_t*),
        int sockType): MXREKernel() {
      sock = zmq::socket_t(ctx, sockType);
      std::string connectingAddr = "tcp://" + addr + ":" + std::to_string(port);
      sock.connect(connectingAddr);

      this->send = send;
      addInputPort<IN_T>("in_data");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("message_sender", std::to_string(pid) + "/message_sender.log");
#endif
    }


    /* Destructor */
    template<typename IN_T>
    MessageSender<IN_T>::~MessageSender() {
      sock.close();
      ctx.shutdown();
      ctx.close();
    }


    /* Run */
    template<typename IN_T>
    raft::kstatus MessageSender<IN_T>::run()
    {
      auto &inData( input["in_data"].template peek<IN_T>() );
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      if(send != NULL) send(&inData, &sock);
      else {
        debug_print("send function pointer is invalid.");
      }
      recyclePort("in_data");
#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif
      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

#endif

