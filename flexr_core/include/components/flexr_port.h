#pragma once

#include <bits/stdc++.h>
#include <raftinc/port.hpp>
#include <flexr_shmq.h>

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


    enum LocalChannel {
      RAFTLIB,
      SHM
    };


    enum RemoteProtocol {
      TCP,
      RTP
    };


    class FleXRPort
    {
      protected:
        bool activated;


        template <typename T>
        bool getInputFromRemote(T* &inputMsg)
        {
          uint8_t *recvBuf   = nullptr;
          uint32_t recvSize  = 0;
          bool isBlocking = true;
          bool received   = false;

          if(inputSemantics == PortDependency::BLOCKING) isBlocking = true;
          else                                           isBlocking = false;

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


        template <typename T>
        bool getInputFromShm(T* &inputMsg)
        {
          uint8_t *recvBuf   = nullptr;
          bool isBlocking = true;
          bool received   = false;

          if(inputSemantics == PortDependency::BLOCKING) isBlocking = true;
          else                                           isBlocking = false;

          recvBuf = new uint8_t[shmPort.elemSize];
          received = shmPort.dequeueElem(recvBuf, shmPort.elemSize, isBlocking);

          if(deserialize != 0 && received)  // deserialize need to set output->data properly & free data
          {
            inputMsg = new T;
            received = deserialize(recvBuf, shmPort.elemSize, (void**)&inputMsg);
          }
          else if(deserialize == 0 && received && shmPort.elemSize == sizeof(T)) // primitive msg (no need deser)
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

        Port              *raftlibPort;
        FlexrShmQueueMeta shmPort;
        ZMQPort           tcpPort;
        RtpPort           rtpPort;

        // Set by deployer
        PortState      state;
        LocalChannel   localChannel;
        RemoteProtocol protocol;

        PortDependency inputSemantics;
        PortDependency outputSemantics;

        std::function<void (FleXRPort*, void*)> sendLocalCopy;

        std::function<bool (void*,      uint8_t* &, uint32_t &, bool)> serialize;
        std::function<bool (uint8_t* &, uint32_t &, void**)>           deserialize;


        FleXRPort(Port* raftlibPort, std::string tag): raftlibPort(raftlibPort), tag(tag), activated(false)
        {
          // Default Settings -- Local
          state         = PortState::LOCAL;
          localChannel  = LocalChannel::RAFTLIB;
          inputSemantics  = PortDependency::BLOCKING;
          outputSemantics = PortDependency::BLOCKING;

          // Default Setting -- remote
          protocol = RemoteProtocol::TCP;

          sendLocalCopy = 0;
          serialize     = 0;
          deserialize   = 0;
        }


        bool checkLocalPortEmpty()
        {
          if(localChannel == LocalChannel::RAFTLIB)
          {
            auto &port((*raftlibPort)[tag]);
            if(port.size() > 0) return false;
            else return true;
          }
          if(localChannel == LocalChannel::RAFTLIB)
          {
            return shmPort.isEmpty();
          }
          return false;
        }


        bool checkLocalPortFull()
        {
          if(localChannel == LocalChannel::RAFTLIB)
          {
            auto &port((*raftlibPort)[tag]);
            if(port.capacity() == port.size()) return true;
            else return false;

          }
          if(localChannel == LocalChannel::RAFTLIB)
          {
            return shmPort.isFull();
          }
          return false;
        }


        bool isActivated() { return activated; }


        /*************** Local Activation  *************/
        template <typename T>
        void activateLocalRaftLibPort(const std::string tag)
        {
          if(activated)
          {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }
          raftlibPort->addPort<T>(tag);
          state = PortState::LOCAL;
          localChannel = LocalChannel::RAFTLIB;
          activated = true;
        }


        void activateLocalShmPort(const std::string tag, const std::string shmId, int shmSize, int shmElemSize)
        {
          if(activated) {
            debug_print("Port %s is already activated.", tag.c_str());
            return;
          }
          shmPort.initQueue(shmId.c_str(), shmSize, shmElemSize);
          state = PortState::LOCAL;
          localChannel = LocalChannel::SHM;
          activated = true;
        }


        template <typename T>
        void activateAsLocalRaftLibInput(const std::string tag)
        {
          activateLocalRaftLibPort<T>(tag);
        }


        void activateAsLocalShmInput(const std::string tag, const std::string shmId, int shmSize, int shmElemSize)
        {
          activateLocalShmPort(tag, shmId, shmSize, shmElemSize);
        }


        template <typename T>
        void activateAsLocalRaftLibOutput(const std::string tag, PortDependency semantics)
        {
          activateLocalRaftLibPort<T>(tag);
          outputSemantics = semantics;
        }


        void activateAsLocalShmOutput(const std::string tag, const std::string shmId, int shmSize, int shmElemSize, PortDependency semantics)
        {
          activateLocalShmPort(tag, shmId, shmSize, shmElemSize);
          outputSemantics = semantics;
        }


        /*************** Remote Activation  *************/
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
            // getInput -- Local
            case PortState::LOCAL:
              // RAFTLIB
              if(localChannel == LocalChannel::RAFTLIB)
              {
                if(inputSemantics == PortDependency::BLOCKING) {
                  T &temp = (*raftlibPort)[tag].peek<T>();
                  input = &temp;
                }
                else if(inputSemantics == PortDependency::NONBLOCKING) {
                  if(checkLocalPortEmpty() == false) {
                    T &temp = (*raftlibPort)[tag].peek<T>();
                    input = &temp;
                  }
                }
              }
              // SHM
              else if(localChannel == LocalChannel::SHM)
              {
                bool received = getInputFromShm(input);
                if(received == false && input != nullptr)
                {
                  delete input;
                  input = nullptr;
                }
              }

              break;

            // getInput -- Remote
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


        template <typename T>
        T* getOutputPlaceholder()
        {
          if(!activated) {
            debug_print("Cannot getOutputPlaceholder with inactivated port %s", tag.c_str());
            return nullptr;
          }

          T *outputPlaceholder = nullptr;

          // getOutputPlaceholder -- local
          if(state == PortState::LOCAL)
          {
            // RaftLib
            if(localChannel == LocalChannel::RAFTLIB)
            {
              T &temp = (*raftlibPort)[tag].allocate<T>();
              outputPlaceholder = &temp;
            }
            // Shm
            else if(localChannel == LocalChannel::SHM)
            {
              outputPlaceholder = new T; // TODO ?!?!?
            }
          }
          // getOutputPlaceholder -- remote
          else if(state == PortState::REMOTE)
          {
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
            // sendOutput -- local
            case PortState::LOCAL:
              // RaftLib
              if(localChannel == LocalChannel::RAFTLIB)
              {
                if(outputSemantics == PortDependency::BLOCKING) {
                  (*raftlibPort)[tag].send();
                }
                else if(outputSemantics == PortDependency::NONBLOCKING) {
                  if(checkLocalPortFull() == false)
                  {
                    (*raftlibPort)[tag].send();
                  }
                }
              }
              // Shm
              else if(localChannel == LocalChannel::SHM)
              {
                sendOutputToShm(msg, true);
              }
              break;
            // sendOutput -- remote
            case PortState::REMOTE:
              sendOutputToRemote(msg, true);
              break;
          }
        }


        template <typename T>
        void sendOutputToShm(T* outputMsg, bool freeMsg)
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

          if(sendSize != shmPort.elemSize)
          {
            debug_print("shm elemSize (%d) is not matched to msg sendSize (%d).", shmPort.elemSize, sendSize);
            if(serialize && sendBuf != nullptr) delete sendBuf;
            if(freeMsg && outputMsg != nullptr) delete outputMsg;
            return;
          }

          if(outputSemantics == PortDependency::BLOCKING) shmPort.enqueueElem(sendBuf, sendSize, true);
          else                                            shmPort.enqueueElem(sendBuf, sendSize, false);
          if(serialize && sendBuf != nullptr) delete sendBuf;
          if(freeMsg && outputMsg != nullptr) delete outputMsg;
        }


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
            if(localChannel == LocalChannel::RAFTLIB) (*raftlibPort)[tag].recycle();
            if(localChannel == LocalChannel::SHM) delete msg; // TODO
            break;
          case PortState::REMOTE:
            delete msg;
            break;
          }
        }
    };
  }
}

