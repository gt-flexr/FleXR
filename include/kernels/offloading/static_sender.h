#ifndef __MXRE_STATIC_SENDER__
#define __MXRE_STATIC_SENDER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <cv.hpp>
#include "defs.h"
#include "message_sender.h"
#include "types/clock_types.h"

namespace mxre
{
  namespace kernels
  {

    /* Class Deifinition */
    template<typename IN_T>
    class StaticSender : public MessageSender
    {
      private:
        bool isVector;
      public:
        StaticSender(std::string addr="localhost", int port=5555, bool isVector=false);
        virtual raft::kstatus run();
    };


    /* Constructor() */
    template<typename IN_T>
    StaticSender<IN_T>::StaticSender(std::string addr, int port, bool isVector) : MessageSender(addr, port) {
      this->isVector = isVector;
      if(isVector)
        input.addPort<std::vector<IN_T>>("in_data");
      else
        input.addPort<IN_T>("in_data");
    }


    /* Run() */
    template<typename IN_T>
    raft::kstatus StaticSender<IN_T>::run() {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      debug_print("[StaticSender] isVector(%d) START", isVector);
      uint numElem, elemSize;
      void *data;
      char ackMsg[4];


      if(isVector) {
        auto &inData( input["in_data"].template peek<std::vector<IN_T>>() );
        numElem = inData.size();
        elemSize = sizeof(IN_T);
        data = inData.data();
      }
      else {
        auto &inData( input["in_data"].template peek<IN_T>() );
        numElem = 1;
        elemSize = sizeof(inData);
        data = &inData;
      }
      debug_print("numElem (%d), elemSize (%d)", numElem, elemSize);

      // 1. Send the num of data & data
      zmq_send(sock, &numElem, sizeof(uint), ZMQ_SNDMORE);
      zmq_send(sock, data, numElem * elemSize, 0);

      // 2. Recv ACK
      zmq_recv(sock, ackMsg, 4, 0);

      input["in_data"].recycle(1);

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

#endif

