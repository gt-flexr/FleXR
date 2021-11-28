#ifndef __FLEXR_CORE_COMPONENTS_PORTMANAGER__
#define __FLEXR_CORE_COMPONENTS_PORTMANAGER__

#include <raftinc/port.hpp>
#include "flexr_core/include/components/flexr_port.h"

namespace flexr
{
  namespace components
  {

    /**
     * @brief Component for managing kernels' local and remote ports with the same APIs
     */
    class FleXRPortManager
    {
      protected:
        Port* inLocalPorts;
        Port* outLocalPorts;

        std::map<std::string, FleXRPort*> inPortMap;
        std::map<std::string, FleXRPort*> outPortMap;
        std::multimap<std::string, std::string> duplicatedOutPortMap;

      public:
        /**
         * @brief Initialize FleXRPortManager with in and out ports
         * @param inLocalPorts
         *  RaftLib kernel's local input ports
         * @param outLocalPorts
         *  RaftLib kernel's local output ports
         */
        FleXRPortManager(Port *inLocalPorts, Port *outLocalPorts): inLocalPorts(inLocalPorts),
                                                                   outLocalPorts(outLocalPorts) {}


        ~FleXRPortManager()
        {
          for(auto const &inPort: inPortMap) delete [] inPort.second;
          for(auto const &outPort: outPortMap) delete [] outPort.second;
        }


        /**
         * @brief Register an input port tag
         * @param tag
         *  Input port tag for the port map
         * @param pd
         *  Input port dependency of flexr::components::PortDependency
         * @param deserializeFunc
         *  Optional function to deserialize received msg to a specific type (not required for primitive types
         */
        void registerInPortTag(std::string tag,
                               PortDependency pd,
                               std::function<bool (uint8_t* &, uint32_t &, void**)> deserializeFunc = 0)
        {
          inPortMap[tag]                 = new FleXRPort(inLocalPorts, tag);
          inPortMap[tag]->dependency     = pd;
          inPortMap[tag]->deserialize    = deserializeFunc;
        }


        /**
         * @brief Register an output port tag
         * @param tag
         *  Output port tag for the port map
         * @param sendLocalCopyFunc
         *  Function to copy an output message for multiple output ports
         * @param serializeFunc
         *  Optional function to serialize a specific type to msg (not required for primitive types)
         */
        void registerOutPortTag(std::string tag,
                                std::function<void (FleXRPort*, void*)> sendLocalCopyFunc,
                                std::function<bool (void*, uint8_t* &, uint32_t &, bool)> serializeFunc = 0)
        {
          outPortMap[tag]                = new FleXRPort(outLocalPorts, tag);
          outPortMap[tag]->sendLocalCopy = sendLocalCopyFunc;
          outPortMap[tag]->serialize     = serializeFunc;
        }


        /**
         * @brief Get an input from a port of the port map with tag
         * @details The port activation determines getting the input locally or remotely
         * @param tag
         *  Tag of the port registered to the port map
         * @return Pointer of the input data
         * @see flexr::components::FleXRPort::getInput
         */
        template <typename T>
        T* getInput(const std::string tag)
        {
          return inPortMap[tag]->getInput<T>();
        }


        /**
         * @brief Get output placeholder from the buffer of the output port
         * @param tag
         *  Tag of the port registered to the port map
         * @return Pointer to the output port buffer
         * @see flexr::components::FleXRPort::getOutputPlaceholder
         */
        template <typename T>
        T* getOutputPlaceholder(const std::string tag)
        {
          return outPortMap[tag]->getOutputPlaceholder<T>();
        }


        /**
         * @brief Get output placeholder from the buffer of the output port
         * @param tag
         *  Tag of the port registered to the port map
         * @return Pointer to the output port buffer
         * @see flexr::components::FleXRPort::getOutputPlaceholder
         */
        template <typename T>
        void sendOutput(std::string tag, T *msg)
        {
          // send dup ports
          auto portRange = duplicatedOutPortMap.equal_range(tag);
          for(auto port = portRange.first; port != portRange.second; ++port)
          {
            switch(outPortMap[port->second]->state)
            {
            case PortState::LOCAL:
              outPortMap[port->second]->sendLocalCopy(outPortMap[port->second], msg);
              break;
            case PortState::REMOTE:
              outPortMap[port->second]->sendOutputToRemote(msg, false);
              break;
            }
          }

          // send original port
          outPortMap[tag]->sendOutput(msg);
        }


        /**
         * @brief Free the received input message
         * @details The input message buffer is the allocated memory of the previous kernle's output port. The data
         *  ownership is moved to the current kernel, and the allocated memory from the pervious kernel is freed by the
         *  current kernel.
         * @param tag
         *  Tag of the port registered to the port map
         * @param msg
         *  Message pointer to free
         * @see flexr::components::FleXRPort::freeInput
         */
        template <typename T>
        void freeInput(std::string tag, T *msg)
        {
          inPortMap[tag]->freeInput(msg);
        }


        /**
         * @brief Activate the registered input port as local
         * @param tag
         *  Tag of the port registered to the port map
         * @see flexr::components::FleXRPort::activateAsLocal
         */
        template <typename T>
        void activateInPortAsLocal(const std::string tag)
        {
          inPortMap[tag]->activateAsLocal<T>(tag);
        }


        /**
         * @brief Activate the registered input port as remote
         * @param tag
         *  Tag of the port registered to the port map
         * @param p
         *  Protocol to use
         * @param portNumber
         *  Port number for receiving input from a remote node
         * @see flexr::components::FleXRPort::activateAsRemoteInput
         */
        template <typename T>
        void activateInPortAsRemote(const std::string tag, RemoteProtocol p, int portNumber)
        {
          inPortMap[tag]->activateAsRemoteInput<T>(p, portNumber);
        }


        /**
         * @brief Activate the registered output port as local
         * @param tag
         *  Tag of the port registered to the port map
         * @see flexr::components::FleXRPort::activateAsLocal
         */
        template <typename T>
        void activateOutPortAsLocal(const std::string tag)
        {
          outPortMap[tag]->activateAsLocal<T>(tag);
        }


        /**
         * @brief Activate the registered output port as remote
         * @param tag
         *  Tag of the port registered to the port map
         * @param p
         *  Protocol to use
         * @param addr
         *  IP address of the destination node to send the output message
         * @param portNumber
         *  Port number of the destination node to send the output message
         * @see flexr::components::FleXRPort::activateAsRemoteInput
         */
        template <typename T>
        void activateOutPortAsRemote(const std::string tag, RemoteProtocol p, const std::string addr, int portNumber)
        {
          outPortMap[tag]->activateAsRemoteOutput<T>(p, addr, portNumber);
        }


        /**
         * @brief Duplicate an activated output port to a new local port with a tag
         * @param originTag
         *  Tag of the activated port in the port map
         * @param newTag
         *  Tag of a new duplicated port to register and activate locally
         * @see registerOutPortTag, activateOutPortAsLocal
         */
        template <typename T>
        void duplicateOutPortAsLocal(const std::string originTag, const std::string newTag)
        {
          duplicatedOutPortMap.insert(std::make_pair<std::string, std::string>(originTag.c_str(), newTag.c_str()));
          registerOutPortTag(newTag,
                             outPortMap[originTag]->sendLocalCopy,
                             outPortMap[originTag]->serialize);
          activateOutPortAsLocal<T>(newTag);
        }


        /**
         * @brief Duplicate an activated output port to a new remote port with a tag
         * @param originTag
         *  Tag of the activated port in the port map
         * @param newTag
         *  Tag of a new duplicated port to register and activate locally
         * @param p
         *  Protocol to use
         * @param addr
         *  IP address of the destination node to send the output message
         * @param portNumber
         *  Port number of the destination node to send the output message
         * @see registerOutPortTag, activateOutPortAsRemote
         */
        template <typename T>
        void duplicateOutPortAsRemote(const std::string originTag, const std::string newTag,
                                      RemoteProtocol p, const std::string addr, const int portNumber)
        {
          duplicatedOutPortMap.insert(std::make_pair<std::string, std::string>(originTag.c_str(), newTag.c_str()));
          registerOutPortTag(newTag,
                             outPortMap[originTag]->sendLocalCopy,
                             outPortMap[originTag]->serialize);
          activateOutPortAsRemote<T>(newTag, p, addr, portNumber);
        }
    };
  }
}

#endif

