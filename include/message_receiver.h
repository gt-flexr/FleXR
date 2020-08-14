#ifndef __MXRE_MSG_RECEIVER__
#define __MXRE_MSG_RECEIVER__

#include <bits/stdc++.h>
#include <raft>

#include <zmq.hpp>

#include "defs.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      template<typename OUT_T, bool isVector=false>
      class MessageReceiver : public raft::kernel
      {
        private:
          zmq::socket_t *sock;

        public:
          MessageReceiver(int port);
          ~MessageReceiver();
          virtual raft::kstatus run();
      };


      template<typename OUT_T, bool isVector>
      MessageReceiver<OUT_T, isVector>::MessageReceiver(int port) {
        std::cout << "MessageReceiver() -- Start" << std::endl;
        zmq::context_t context(1);
        sock = new zmq::socket_t(context, ZMQ_REP);
        std::string targetAddr = "tcp://*:" + std::to_string(port);
        std::cout << targetAddr << std::endl;
        sock->bind(targetAddr.c_str());

        output.addPort<OUT_T>("out_data");
        std::cout << "MessageReceiver() -- Done" << std::endl;
        return;
      }

      template<typename OUT_T, bool isVector>
      MessageReceiver<OUT_T, isVector>::~MessageReceiver() {
        delete[] sock;
      }

      template<typename OUT_T, bool isVector>
      raft::kstatus MessageReceiver<OUT_T, isVector>::run() {
        std::cout << "[MessageReceiver] run" << std::endl;

        // ack msg
        zmq::message_t ackMsg(3);
        memcpy(ackMsg.data(), "ACK", 3);

        // recv msgs
        zmq::message_t numMsg, *dataMsgs;
        sock->recv(numMsg);
        uint *num = static_cast<uint*>(numMsg.data());
        dataMsgs = new zmq::message_t[*num];

        if(isVector) {
          for(uint i = 0; i < *num; i++) {
            sock->recv(dataMsgs[i]);
            OUT_T *receivedData = static_cast<OUT_T*>(dataMsgs[i].data());
            auto outData = output["out_data"].template allocate_s<std::vector<OUT_T>>();
            (*outData).push_back(*receivedData);
          }
        }
        else {
          sock->recv(dataMsgs[0]);
          OUT_T *receivedData = static_cast<OUT_T*>(dataMsgs[0].data());
          auto outData = output["out_data"].template allocate_s<OUT_T>();
          *outData = *receivedData;
        }

        sock->send(ackMsg);
        output["out_data"].send();
        delete[] dataMsgs; // what happen if I delete this...?
        return raft::proceed;
      }
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif
