#ifndef __MXRE_COMPONENTS_MXREPORT__
#define __MXRE_COMPONENTS_MXREPORT__

#include <bits/stdc++.h>
#include <raftinc/port.hpp>

#include "components/zmq_port.h"
#include "defs.h"

namespace mxre
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

    class MXREPort
    {
      protected:
        bool activated;
        bool checkPort()
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

        std::function<void (MXREPort*, void*)> sendLocalCopy;
        std::function<void (MXREPort*, void*)> sendRemote;
        std::function<void (void*)>            freeRemoteMsg;
        std::function<void (MXREPort*, void*)> recvRemote;

        MXREPort(Port* localPort, std::string tag): localPort(localPort), tag(tag), activated(false)
        { }

        bool isActivated() { return activated; }

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
              if(checkPort()) {
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
            break;
          }
        }

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
            delete [] msg;
            break;
          }
        }
    };
  }
}

#endif

