#ifndef __MXRE_COMPONENTS_MXREPORTMANAGER__
#define __MXRE_COMPONENTS_MXREPORTMANAGER__

#include <raftinc/port.hpp>
#include "components/mxre_port.h"

namespace mxre
{
  namespace components
  {
    class MXREPortManager
    {
      protected:
        Port* inLocalPorts;
        Port* outLocalPorts;

        std::map<std::string, MXREPort*> inPortMap;
        std::map<std::string, MXREPort*> outPortMap;
        std::multimap<std::string, std::string> duplicatedOutPortMap;

      public:
        MXREPortManager(Port *inLocalPorts, Port *outLocalPorts): inLocalPorts(inLocalPorts),
                                                                  outLocalPorts(outLocalPorts)
        {}

        ~MXREPortManager()
        {
          for(auto const &inPort: inPortMap) delete [] inPort.second;
          for(auto const &outPort: outPortMap) delete [] outPort.second;
        }

        /*
         * Interfaces for Kernel Developers
         *   - registerInPortTag
         *   - registerOutPortTag
         *   - getInput
         *   - getInputWithSize
         *   - getOutputPlaceholder
         *   - freeInput
         */
        void registerInPortTag(std::string tag, PortDependency pd,
                               std::function<void (MXREPort*, void*)> recvRemoteFunc,
                               std::function<void (void*, int)> allocRemoteMsgFunc = 0)
        {
          inPortMap[tag]                 = new MXREPort(inLocalPorts, tag);
          inPortMap[tag]->dependency     = pd;
          inPortMap[tag]->recvRemote     = recvRemoteFunc;
          inPortMap[tag]->allocRemoteMsg = allocRemoteMsgFunc;
        }

        void registerOutPortTag(std::string tag,
                                std::function<void (MXREPort*, void*)> sendLocalCopyFunc,
                                std::function<void (MXREPort*, void*)> sendRemoteFunc,
                                std::function<void (void*)> freeRemoteMsgFunc)
        {
          outPortMap[tag]                = new MXREPort(outLocalPorts, tag);
          outPortMap[tag]->sendLocalCopy = sendLocalCopyFunc;
          outPortMap[tag]->sendRemote    = sendRemoteFunc;
          outPortMap[tag]->freeRemoteMsg = freeRemoteMsgFunc;
        }


        template <typename T>
        T* getInput(const std::string tag)
        {
          return inPortMap[tag]->getInput<T>();
        }


        template <typename T>
        T* getInputWithSize(const std::string tag, int size)
        {
          return inPortMap[tag]->getInputWithSize<T>(size);
        }


        template <typename T>
        T* getOutputPlaceholder(const std::string tag)
        {
          return outPortMap[tag]->getOutputPlaceholder<T>();
        }

        template <typename T>
        void sendOutput(std::string tag, T *msg)
        {
          auto portRange = duplicatedOutPortMap.equal_range(tag);
          for(auto port = portRange.first; port != portRange.second; ++port) {
            switch(outPortMap[port->second]->state) {
            case PortState::LOCAL:
              outPortMap[port->second]->sendLocalCopy(outPortMap[port->second], msg);
              break;
            case PortState::REMOTE:
              outPortMap[port->second]->sendOutput(msg);
              break;
            }
          }

          outPortMap[tag]->sendOutput(msg);
          if(outPortMap[tag]->state == PortState::REMOTE) outPortMap[tag]->freeRemoteMsg(msg);
        }

        template <typename T>
        void freeInput(std::string tag, T *msg)
        {
          inPortMap[tag]->freeInput(msg);
        }

        /*
         * Interfaces for Kernel Deployers
         *   - activateInPortAsLocal
         *   - activateInPortAsRemote
         *   - activateOutPortAsLocal
         *   - activateOutPortAsRemote
         *   - duplicateOutPortAsLocal
         *   - duplicateOutPortAsRemote
         */
        template <typename T>
        void activateInPortAsLocal(const std::string tag)
        {
          inPortMap[tag]->activateAsLocal<T>(tag);
        }

        template <typename T>
        void activateInPortAsRemote(const std::string tag, int portNumber)
        {
          inPortMap[tag]->activateAsRemoteInput<T>(portNumber);
        }

        template <typename T>
        void activateOutPortAsLocal(const std::string tag)
        {
          outPortMap[tag]->activateAsLocal<T>(tag);
        }

        template <typename T>
        void activateOutPortAsRemote(const std::string tag, const std::string addr, int portNumber)
        {
          outPortMap[tag]->activateAsRemoteOutput<T>(addr, portNumber);
        }

        template <typename T>
        void duplicateOutPortAsLocal(const std::string originTag, const std::string newTag)
        {
          duplicatedOutPortMap.insert(std::make_pair<std::string, std::string>(originTag.c_str(), newTag.c_str()));
          registerOutPortTag(newTag,
                             outPortMap[originTag]->sendLocalCopy,
                             outPortMap[originTag]->sendRemote,
                             outPortMap[originTag]->freeRemoteMsg);
          activateOutPortAsLocal<T>(newTag);
        }

        template <typename T>
        void duplicateOutPortAsRemote(const std::string originTag, const std::string newTag,
                                      const std::string addr, const int portNumber)
        {
          duplicatedOutPortMap.insert(std::make_pair<std::string, std::string>(originTag.c_str(), newTag.c_str()));
          registerOutPortTag(newTag,
                             outPortMap[originTag]->sendLocalCopy,
                             outPortMap[originTag]->sendRemote,
                             outPortMap[originTag]->freeRemoteMsg);
          activateOutPortAsRemote<T>(newTag, addr, portNumber);
        }
    };
  }
}

#endif

