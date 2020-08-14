#ifndef __MXRE_MSG_SENDER__
#define __MXRE_MSG_SENDER__

#include <bits/stdc++.h>
#include <raft>

#include <zmq.hpp>

#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      template<typename IN_T, bool isVector=false>
      class MessageSender : public raft::kernel
      {
        private:
          zmq::socket_t *sock;
          size_t sendDataSize;
        public:
          MessageSender(std::string addr="localhost", int port=5555, size_t dataSize=128);
          ~MessageSender();
          virtual raft::kstatus run();
      };


      template<typename IN_T, bool isVector>
      MessageSender<IN_T, isVector>::MessageSender(std::string addr, int port, size_t dataSize) : raft::kernel() {
        zmq::context_t context(1);
        sock = new zmq::socket_t(context, ZMQ_REQ);
        sendDataSize = dataSize;
        std::string targetAddr = "tcp://" + addr + ":" + std::to_string(port);
        std::cout << targetAddr << std::endl;
        sock->connect(targetAddr.c_str());

        if(isVector)
          input.addPort<std::vector<IN_T>>("in_data");
        else
          input.addPort<IN_T>("in_data");
      }

      template<typename IN_T, bool isVector>
      MessageSender<IN_T, isVector>::~MessageSender() {
        delete [] sock;
      }

      template<typename IN_T, bool isVector>
      raft::kstatus MessageSender<IN_T, isVector>::run() {
        std::cout << "[MessageSender] run" << std::endl;
        zmq::message_t ackMsg;
        zmq::message_t numMsg(sizeof(uint));
        zmq::message_t dataMsg(sendDataSize);

        // send data
        if(isVector) {
          auto inData = input["in_data"].template peek<std::vector<IN_T>>();
          uint num = inData.size();

          // send # of vector elems
          memcpy(numMsg.data(), &num, sizeof(uint));
          sock->send(numMsg, ZMQ_SNDMORE);

          typename std::vector<IN_T>::iterator iter;
          for(iter = inData.begin(); iter != inData.end(); ++iter) {
            memcpy(dataMsg.data(), &*iter, sendDataSize);
            if(iter == std::prev(inData.end()))
              sock->send(dataMsg);
            else
              sock->send(dataMsg, ZMQ_SNDMORE);
          }
        }
        else {
          auto inData = input["in_data"].template peek<IN_T>();
          uint num = 1;
          memcpy(numMsg.data(), &num, sizeof(uint));
          sock->send(numMsg, ZMQ_SNDMORE);

          memcpy(dataMsg.data(), &inData, sendDataSize);
          sock->send(dataMsg);
        }

        // recv ack
        sock->recv(&ackMsg);
        return raft::proceed;
      }
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif
