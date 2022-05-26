#ifndef __FLEXR_CORE_COMPONENTS_PORTMANAGER__
#define __FLEXR_CORE_COMPONENTS_PORTMANAGER__

#include <raftinc/port.hpp>
#include "flexr_core/include/components/flexr_port.h"
#include "flexr_core/include/yaml_ports.h"

namespace flexr
{
  namespace components
  {

    class FleXRPortManager
    {
      protected:
        // RaftLib Ports for initialization
        Port* inLocalPorts;
        Port* outLocalPorts;

        std::map<std::string, FleXRPort*> inPortMap;
        std::map<std::string, FleXRPort*> outPortMap;
        std::multimap<std::string, std::string> duplicatedOutPortMap;


      public:
        FleXRPortManager(Port *inLocalPorts, Port *outLocalPorts): inLocalPorts(inLocalPorts),
                                                                   outLocalPorts(outLocalPorts) {}


        ~FleXRPortManager()
        {
          for(auto const &inPort: inPortMap) delete [] inPort.second;
          for(auto const &outPort: outPortMap) delete [] outPort.second;
        }


        /***** Port Registration -- Developer *****/
        void registerInPortTag(std::string tag,
                               PortDependency pd,
                               std::function<bool (uint8_t* &, uint32_t &, void**)> deserializeFunc = 0)
        {
          inPortMap[tag]                 = new FleXRPort(inLocalPorts, tag);
          inPortMap[tag]->inputSemantics = pd; // inputPortDependency is set by developer...
          inPortMap[tag]->deserialize    = deserializeFunc;
        }


        void registerOutPortTag(std::string tag,
                                std::function<void (FleXRPort*, void*)> sendLocalCopyFunc,
                                std::function<bool (void*, uint8_t* &, uint32_t &, bool)> serializeFunc = 0)
        {
          outPortMap[tag]                  = new FleXRPort(outLocalPorts, tag);
          outPortMap[tag]->sendLocalCopy   = sendLocalCopyFunc;
          outPortMap[tag]->serialize       = serializeFunc;
        }


        /***** Port Activation -- Deployer *****/
        // In Ports
        template <typename T>
        void activateInPortAsLocal(yaml::YamlInPort inPort)
        {
          // RaftLib Activation
          if(inPort.localChannel == "raftlib")
          {
            inPortMap[inPort.portName]->activateAsLocalRaftLibInput<T>(inPort.portName);
          }
          // Shm Activation
          else if(inPort.localChannel == "shm")
          {
            if(inPort.shmSize < 0) inPort.shmSize = 1;

            if(inPort.shmMaxElemSize < sizeof(T))
            {
              debug_print("shmMaxElemSize (%d) is less than the size of primitive type (%d)", inPort.shmMaxElemSize, sizeof(T));
              exit(1);
            }

            inPortMap[inPort.portName]->activateAsLocalShmInput(inPort.portName, inPort.shmId, inPort.shmSize, inPort.shmMaxElemSize);
          }
        }


        template <typename T>
        void activateInPortAsRemote(yaml::YamlInPort inPort)
        {
          components::RemoteProtocol p = components::RemoteProtocol::TCP;
          if(inPort.protocol == std::string("TCP"))
          {
            p = components::RemoteProtocol::TCP;
          }
          else if(inPort.protocol == std::string("RTP"))
          {
            p = components::RemoteProtocol::RTP;
          }

          inPortMap[inPort.portName]->activateAsRemoteInput(p, inPort.bindingPortNum);
        }


        // Out Ports
        template <typename T>
        void activateOutPortAsLocal(yaml::YamlOutPort outPort)
        {
          // Set output semantics
          components::PortDependency pd;
          if(outPort.semantics == "blocking") pd = components::PortDependency::BLOCKING;
          else pd = components::PortDependency::NONBLOCKING;

          // RaftLib Activation
          if(outPort.localChannel == "raftlib")
          {
            outPortMap[outPort.portName]->activateAsLocalRaftLibOutput<T>(outPort.portName, pd);
          }
          // Shm Activation
          else if(outPort.localChannel == "shm")
          {
            if(outPort.shmSize < 0) outPort.shmSize = 1;

            debug_print("outPort.shmMaxElemSize - before: %d", outPort.shmMaxElemSize);
            if(outPort.shmMaxElemSize < sizeof(T))
            {
              debug_print("shmMaxElemSize (%d) is less than the size of primitive type (%d)", outPort.shmMaxElemSize, sizeof(T));
              exit(1);
            }

            outPortMap[outPort.portName]->activateAsLocalShmOutput(outPort.portName, outPort.shmId, outPort.shmSize, outPort.shmMaxElemSize, pd);
          }
        }


        void activateOutPortAsRemote(yaml::YamlOutPort outPort)
        {
          components::RemoteProtocol p = components::RemoteProtocol::TCP;
          if(outPort.protocol == std::string("TCP"))
          {
            p = components::RemoteProtocol::TCP;
          }
          else if(outPort.protocol == std::string("RTP"))
          {
            p = components::RemoteProtocol::RTP;
          }

          outPortMap[outPort.portName]->activateAsRemoteOutput(p, outPort.connectingAddr, outPort.connectingPortNum);
        }


        /***** Port Duplication -- Deployer *****/
        template <typename T>
        void duplicateOutPortAsLocal(yaml::YamlOutPort outPort)
        {
          duplicatedOutPortMap.insert(std::make_pair<std::string, std::string>(outPort.duplicatedFrom.c_str(), outPort.portName.c_str()));
          registerOutPortTag(outPort.portName,
                             outPortMap[outPort.duplicatedFrom]->sendLocalCopy,
                             outPortMap[outPort.duplicatedFrom]->serialize);
          activateOutPortAsLocal<T>(outPort);
        }


        template <typename T>
        void duplicateOutPortAsRemote(yaml::YamlOutPort outPort)

        {
          duplicatedOutPortMap.insert(std::make_pair<std::string, std::string>(outPort.duplicatedFrom.c_str(), outPort.portName.c_str()));
          registerOutPortTag(outPort.portName,
                             outPortMap[outPort.duplicatedFrom]->sendLocalCopy,
                             outPortMap[outPort.duplicatedFrom]->serialize);
          activateOutPortAsRemote(outPort);
        }


        /***** Port Interfaces -- Developer *****/
        template <typename T>
        T* getInput(const std::string tag)
        {
          return inPortMap[tag]->getInput<T>();
        }


        template <typename T>
        T* getOutputPlaceholder(const std::string tag)
        {
          return outPortMap[tag]->getOutputPlaceholder<T>();
        }


        template <typename T>
        void sendOutput(std::string tag, T *msg)
        {
          // send dup ports
          auto portRange = duplicatedOutPortMap.equal_range(tag);
          for(auto port = portRange.first; port != portRange.second; ++port)
          {
            switch(outPortMap[port->second]->state)
            {
            // sendOutput -- local
            case PortState::LOCAL:
              // RaftLib Port
              if(outPortMap[port->second]->localChannel == LocalChannel::RAFTLIB)
              {
                if(outPortMap[port->second]->outputSemantics == PortDependency::BLOCKING)
                {
                  outPortMap[port->second]->sendLocalCopy(outPortMap[port->second], msg);
                }
                else if(outPortMap[port->second]->outputSemantics == PortDependency::NONBLOCKING)
                {
                  if(outPortMap[port->second]->checkLocalPortFull() == false)
                  {
                    outPortMap[port->second]->sendLocalCopy(outPortMap[port->second], msg);
                  }
                }
              }
              // Shm Port
              else if(outPortMap[port->second]->localChannel == LocalChannel::SHM)
              {
                outPortMap[port->second]->sendOutputToShm(msg, false);
              }
              break;

            // sendOutput -- remote
            case PortState::REMOTE:
              outPortMap[port->second]->sendOutputToRemote(msg, false);
              break;
            }
          }

          // send original port
          outPortMap[tag]->sendOutput(msg);
        }


        template <typename T>
        void freeInput(std::string tag, T *msg)
        {
          inPortMap[tag]->freeInput(msg);
        }


    };
  }
}

#endif

