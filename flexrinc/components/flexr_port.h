#ifndef __FLEXR_COMPONENTS_FLEXRPORT__
#define __FLEXR_COMPONENTS_FLEXRPORT__

#include <bits/stdc++.h>
#include <raftinc/port.hpp>

#include "components/zmq_port.h"
#include "components/rtp_port.h"
#include "defs.h"

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
        bool getInputFromRemote(T* &input)
        {
          uint8_t *data   = nullptr;
          uint32_t size   = 0;
          bool isBlocking = true;
          bool received   = false;

          if(dependency == PortDependency::BLOCKING)
            isBlocking = true;
          else
            isBlocking = false;

          if(protocol == RemoteProtocol::TCP)
          {
            debug_print("getInputFromRemote TCP");
            received = tcpPort.receiveMsg(isBlocking, data, size, input->ts);
          }

          if(protocol == RemoteProtocol::RTP)
          {
            debug_print("getInputFromRemote RTP");
            received = rtpPort.receiveMsg(isBlocking, data, size, input->ts);
          }

          if(deserialize != 0 && received)
          {
            deserialize(data, size, (void**)&input); // deserialize need to set output->data properly & free data
            received = true;
          }
          if(received == false)
          {
            debug_print("Failed to receive");
          }

          return received;
        }


        /**
         * @brief Send output msg via internal network ports
         * @param output
         *  output message pointer to send src
         */
        template <typename T>
        void sendOutputToRemote(T *output)
        {
          uint8_t *data     = nullptr;
          uint32_t size     = 0;

          if(serialize != 0)
          {
            serialize((void*)output, data, size); // serialize need to free output->data
          }
          else
          {
            // static msg: ex) T = Message<int>, Message<int[100]> ...
            data         = (uint8_t*)output;
            size         = sizeof(T);
          }

          if(protocol == RemoteProtocol::TCP)
          {
            zmq::message_t sendMsg(data, size);
            tcpPort.socket.send(sendMsg, zmq::send_flags::none); // BK
          }

          if(protocol == RemoteProtocol::RTP)
          {
            rtpPort.send(data, size, output->ts); // NBK..., need to make BK
          }

          // delete serialized buffer after sending
          if(serialize) delete data;
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

        std::function<void (void*,      uint8_t* &, uint32_t &)> serialize;
        std::function<void (uint8_t* &, uint32_t &,     void**)> deserialize;

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
              input = new T;
              if(getInputFromRemote(input) == false) // for non-blocking recv
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
            sendOutputToRemote(msg);
            break;
          }
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

#endif

