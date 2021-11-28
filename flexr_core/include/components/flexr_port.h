#pragma once

#include <bits/stdc++.h>
#include <raftinc/port.hpp>

#include "flexr_core/include/components/zmq_port.h"
#include "flexr_core/include/components/rtp_port.h"
#include "flexr_core/include/defs.h"

namespace flexr
{
  namespace components
  {

    enum PortDependency {
      BLOCKING,
      NONBLOCKING
    };


    enum PortState {
      LOCAL,
      REMOTE
    };


    enum RemoteProtocol {
      TCP,
      RTP
    };


    /**
     * @brief Component for remote and local communications of kernels
     */
    class FleXRPort
    {
      protected:
        bool activated;


        /**
         * @brief Check the existence of an activated local port
         * @return True if there is
         */
        bool checkLocalPort()
        {
          auto &port((*localPort)[tag]);
          if(port.size() > 0) return true;
          else return false;
        }


        /**
         * @brief Get input msg from internal network ports
         * @param input
         *  input message pointer to recv dst
         */
        template <typename T>
        bool getInputFromRemote(T* &inputMsg)
        {
          uint8_t *recvBuf   = nullptr;
          uint32_t recvSize  = 0;
          bool isBlocking = true;
          bool received   = false;

          if(dependency == PortDependency::BLOCKING) isBlocking = true;
          else                                       isBlocking = false;

          if(protocol == RemoteProtocol::TCP)
          {
            received = tcpPort.receiveMsg(isBlocking, recvBuf, recvSize);
          }
          if(protocol == RemoteProtocol::RTP)
          {
            received = rtpPort.receiveMsg(isBlocking, recvBuf, recvSize);
          }

          if(deserialize != 0 && received)  // deserialize need to set output->data properly & free data
          {
            inputMsg = new T;
            received = deserialize(recvBuf, recvSize, (void**)&inputMsg);
          }
          else if(deserialize == 0 && received && recvSize == sizeof(T)) // primitive msg (no need deser)
          {
            inputMsg = (T*)recvBuf;
            recvBuf = nullptr;
          }
          else
          {
            debug_print("received failed...");
          }

          if(recvBuf != nullptr) delete recvBuf;
          return received;
        }


      public:
        std::string tag;

        Port           *localPort;
        ZMQPort        tcpPort;
        RtpPort        rtpPort;

        // Set by developer
        PortDependency dependency;

        // Set by deployer
        PortState      state;
        RemoteProtocol protocol;

        std::function<void (FleXRPort*, void*)> sendLocalCopy;

        std::function<bool (void*,      uint8_t* &, uint32_t &, bool)> serialize;
        std::function<bool (uint8_t* &, uint32_t &, void**)> deserialize;


        /**
         * @brief Check the existence of an activated local port
         * @param localPort
         *  Local port array of Raftlib kernel
         * @param tag
         *  Tag of this port
         */
        FleXRPort(Port* localPort, std::string tag): localPort(localPort), tag(tag), activated(false)
        {
          sendLocalCopy = 0;
          serialize     = 0;
          deserialize   = 0;
        }


        /**
         * @brief Check the activation of the port
         * @return True if the port is activated
         */
        bool isActivated() { return activated; }


        /**
         * @brief Activate and instantiate a local port with the given tag
         * @details Local activation interface is used for both input and output ports
         * @param Tag
         *  Port tag to activate as local
         */
        template <typename T>
        void activateAsLocal(const std::string tag)
        {
          if(activated) {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }
          localPort->addPort<T>(tag);
          state = PortState::LOCAL;
          activated = true;
        }


        /**
         * @brief Activate an input port as remote and bind to a port
         * @param p
         *  Protocol to use
         * @param portNumber
         *  Port number to bind (listen)
         */
        template <typename T>
        void activateAsRemoteInput(RemoteProtocol p,  int portNumber)
        {
          if(activated) {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }

          if(p == RemoteProtocol::TCP)
          {
            tcpPort.bind(portNumber);
            protocol = RemoteProtocol::TCP;
          }
          else if(p == RemoteProtocol::RTP)
          {
            rtpPort.init("127.0.0.1", portNumber, -1);
            protocol = RemoteProtocol::RTP;
          }

          state = PortState::REMOTE;
          activated = true;
        }


        /**
         * @brief Activate an output port as remote and connect to remote node
         * @param p
         *  Protocol to use
         * @param addr
         *  IP address to connect
         * @param portNumber
         *  Port number to connect
         */
        template <typename T>
        void activateAsRemoteOutput(RemoteProtocol p, const std::string addr, int portNumber)
        {
          if(activated) {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }

          if(p == RemoteProtocol::TCP)
          {
            tcpPort.connect(addr, portNumber);
            protocol = RemoteProtocol::TCP;
          }
          else if(p == RemoteProtocol::RTP)
          {
            rtpPort.init(addr, -1, portNumber);
            protocol = RemoteProtocol::RTP;
          }

          state = PortState::REMOTE;
          activated = true;
        }


        /**
         * @brief Get input message
         * @return Pointer to the input message
         */
        template <typename T>
        T* getInput()
        {
          if(!activated) {
            debug_print("Cannot getInput with inactivated port %s", tag.c_str());
            return nullptr;
          }

          T* input = nullptr;
          switch(state)
          {
            case PortState::LOCAL:
              if(dependency == PortDependency::BLOCKING) {
                T &temp = (*localPort)[tag].peek<T>();
                input = &temp;
              }
              else if(dependency == PortDependency::NONBLOCKING) {
                if(checkLocalPort()) {
                  T &temp = (*localPort)[tag].peek<T>();
                  input = &temp;
                }
              }
              break;

            case PortState::REMOTE:
              bool received = getInputFromRemote(input);
              if(received == false && input != nullptr) // for failure
              {
                delete input;
                input = nullptr;
              }
              break;
          }

          return input;
        }


        /**
         * @brief Get the placeholder of an output message
         * @return Pointer to the output message placeholder
         */
        template <typename T>
        T* getOutputPlaceholder()
        {
          if(!activated) {
            debug_print("Cannot getOutputPlaceholder with inactivated port %s", tag.c_str());
            return nullptr;
          }

          T *outputPlaceholder = nullptr;

          if(state == PortState::LOCAL)
          {
            T &temp = (*localPort)[tag].allocate<T>();
            outputPlaceholder = &temp;
          }
          else if(state == PortState::REMOTE)
          {
            outputPlaceholder = new T;
          }

          return outputPlaceholder;
        }


        /**
         * @brief Send the output message of the placeholder
         * @param msg
         *  Pointer to the output message to send
         */
        template <typename T>
        void sendOutput(T* msg)
        {
          if(!activated) {
            debug_print("Cannot sendOutput with inactivated port %s", tag.c_str());
            return;
          }

          switch(state) {
          case PortState::LOCAL:
            (*localPort)[tag].send();
            break;
          case PortState::REMOTE:
            sendOutputToRemote(msg, true);
            break;
          }
        }


        /**
         * @brief Send output msg via internal network ports
         * @param output
         *  output message pointer to send src
         */
        template <typename T>
        void sendOutputToRemote(T *outputMsg, bool freeMsg)
        {
          uint8_t *sendBuf  = nullptr;
          uint32_t sendSize = 0;

          if(serialize != 0)
          {
            // serialize need to free output->data when freeMsgData==true
            bool freeMsgData = freeMsg;
            serialize((void*)outputMsg, sendBuf, sendSize, freeMsgData);
          }
          else
          {
            // static msg: ex) T = Message<int>, Message<int[100]> ...
            sendBuf  = (uint8_t*)outputMsg;
            sendSize = sizeof(T);
          }

          if(protocol == RemoteProtocol::TCP)
          {
            zmq::message_t zmqMsg(sendBuf, sendSize);
            tcpPort.socket.send(zmqMsg, zmq::send_flags::none); // relaxed BK with high-water mark
          }

          if(protocol == RemoteProtocol::RTP)
          {
            rtpPort.send(sendBuf, sendSize, outputMsg->ts); // NBK..., need to make BK
          }

          // delete serialized buffer after sending & conditionally output also
          if(serialize && sendBuf != nullptr) delete sendBuf;
          if(freeMsg && outputMsg != nullptr) delete outputMsg;
        }


        /**
         * @brief Free the memory of an input message
         * @param msg
         *  Pointer to the input message to free
         */
        template <typename T>
        void freeInput(T* msg)
        {
          if(!activated) {
            debug_print("Cannot freeInput with inactivated port %s", tag.c_str());
            return;
          }

          if(msg == nullptr) return;

          switch(state) {
          case PortState::LOCAL:
            (*localPort)[tag].recycle();
            break;
          case PortState::REMOTE:
            delete msg;
            break;
          }
        }
    };
  }
}

