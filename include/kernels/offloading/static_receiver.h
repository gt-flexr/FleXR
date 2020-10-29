#ifndef __MXRE_STATIC_RECEIVER__
#define __MXRE_STATIC_RECEIVER__

#include <bits/stdc++.h>
#include <raft>
#include <utility>
#include <zmq.h>
#include <cv.hpp>
#include "defs.h"
#include "message_receiver.h"
#include "types/clock_types.h"

// pinter values into vector: https://bit.ly/317bzfz

namespace mxre
{
  namespace kernels
  {

    /* Class Deifinition */
    template<typename OUT_T>
    class StaticReceiver : public MessageReceiver
    {
      private:
        bool isVector;
      public:
        StaticReceiver(int port=5555, bool isVector=false);
        virtual raft::kstatus run();
    };


    /* Constructor() */
    template<typename OUT_T>
    StaticReceiver<OUT_T>::StaticReceiver(int port, bool isVector) : MessageReceiver(port) {
      this->isVector = isVector;
      if(isVector)
        output.addPort<std::vector<OUT_T>>("out_data");
      else
        output.addPort<OUT_T>("out_data");
    }


    /* Run() */
    template<typename OUT_T>
    raft::kstatus StaticReceiver<OUT_T>::run() {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      debug_print("[StaticReceiver] isVector(%d) START", isVector);
      uint numElem, elemSize;
      void *data;
      char ackMsg[4];
      memcpy(ackMsg, "ACK\0", 4);


      // 1. Recv num of element
      zmq_recv(sock, &numElem, sizeof(uint), 0);

      // 2. Set recv data info & Recv data
      auto &outData( output["out_data"].template allocate<std::vector<OUT_T>>() );
      if(isVector) {
        //auto &outData( output["out_data"].template allocate<std::vector<OUT_T>>() );
        elemSize = sizeof(OUT_T);
        outData.resize(numElem);
        data = outData.data();
      }
      else {
        auto &outData( output["out_data"].template allocate<OUT_T>() );
        elemSize = sizeof(outData);
        data = &outData;
      }
      debug_print("numElem (%d), elemSize (%d)", numElem, elemSize);
      zmq_recv(sock, data, elemSize*numElem, 0);

      // 3. Send ACK
      zmq_send(sock, ackMsg, 4, 0);


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

