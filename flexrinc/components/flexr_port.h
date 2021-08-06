#ifndef __FLEXR_COMPONENTS_FLEXRPORT__
#define __FLEXR_COMPONENTS_FLEXRPORT__

#include <bits/stdc++.h>
#include <raftinc/port.hpp>

#include "components/zmq_port.h"
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


      public:
        std::string tag;
        Port           *localPort;
        ZMQPort        remotePort;
        PortDependency dependency;
        PortState      state;

        std::function<void (FleXRPort*, void*)> sendLocalCopy;
        std::function<void (FleXRPort*, void*)> sendRemote;
        std::function<void (void*)>            freeRemoteMsg;
        std::function<void (FleXRPort*, void*)> recvRemote;
        std::function<void (void**, int)>      allocRemoteMsg;


        /**
         * @brief Check the existence of an activated local port
         * @param localPort
         *  Local port array of Raftlib kernel
         * @param tag
         *  Tag of this port
         */
        FleXRPort(Port* localPort, std::string tag): localPort(localPort), tag(tag), activated(false)
        { }


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
         * @param portNumber
         *  Port number to bind (listen)
         */
        template <typename T>
        void activateAsRemoteInput(int portNumber)
        {
          if(activated) {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }
          remotePort.bind(portNumber);
          state = PortState::REMOTE;
          activated = true;
        }


        /**
         * @brief Activate an output port as remote and connect to remote node
         * @param addr
         *  IP address to connect
         * @param portNumber
         *  Port number to connect
         */
        template <typename T>
        void activateAsRemoteOutput(const std::string addr, int portNumber)
        {
          if(activated) {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }
          remotePort.connect(addr, portNumber);
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
          switch(state) {
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
            // currently, REMOTE port only works in blocking mode.
            input = new T;
            recvRemote(this, input);
            break;
          }
          return input;
        }


        /**
         * @brief Get input message with the given data size
         * @param size
         *  Size of the input message to get
         * @return Pointer to the input message
         */
        template <typename T>
        T* getInputWithSize(int size)
        {
          if(!activated) {
            debug_print("Cannot getInput with inactivated port %s", tag.c_str());
            return nullptr;
          }

          T* input = nullptr;
          switch(state) {
          case PortState::LOCAL:
            input = getInput<T>();
            break;
          case PortState::REMOTE:
            if(allocRemoteMsg) allocRemoteMsg((void**)&input, size);
            else input = new T;
            recvRemote(this, input);
            break;
          }
          return input;
        }


        /**
         * @brief Get the placeholder of an output message
         * @return Pointer to the output message
         */
        template <typename T>
        T* getOutputPlaceholder()
        {
          if(!activated) {
            debug_print("Cannot getOutputPlaceholder with inactivated port %s", tag.c_str());
            return nullptr;
          }

          T *outputPlaceholder = nullptr;
          if(state == PortState::LOCAL) {
            T &temp = (*localPort)[tag].allocate<T>();
            outputPlaceholder = &temp;
          }
          else if(state == PortState::REMOTE) {
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
            sendRemote(this, msg);
            //if(freeRemoteMsg) freeRemoteMsg(msg);
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
            // delete[] vs delete: not expect to call each element desctructor
            delete msg;
            break;
          }
        }
    };
  }
}

#endif

