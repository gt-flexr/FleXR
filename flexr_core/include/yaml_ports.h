#ifndef __FLEXR_CORE_YAML_PORTS__
#define __FLEXR_CORE_YAML_PORTS__

#include <bits/stdc++.h>
#include "flexr_core/include/defs.h"

namespace flexr
{
  namespace yaml
  {
    class YamlPort
    {
    public:
      std::string portName;        // tag
      std::string connectionType;  // state

      // local
      std::string localChannel;    // localChannel
      std::string shmId;           // shmPort ID
      int shmSize;                 // shmPort queue size
      int shmElemSize;             // shmPort elem size

      // remote
      std::string protocol;

      YamlPort()
      {
        portName       = "";
        connectionType = "";

        // local
        localChannel   = "";
        shmId          = "";
        shmSize        = -1;
        shmElemSize    = -1;

        // remote
        protocol       = "";
      }

      void printBase()
      {
        std::cout << "PortBase -------- " << std::endl;
        std::cout << "\tPort Name: " << portName << std::endl;
        std::cout << "\tConnection Type: " << connectionType << std::endl;
        if(connectionType == std::string("local"))
        {
          std::cout << "\tLocal Channel: " << localChannel << std::endl;
          if(localChannel == std::string("shm"))
          {
            std::cout << "\tShmID:       " << shmId       << std::endl;
            std::cout << "\tShmSize:     " << shmSize     << std::endl;
            std::cout << "\tShmElemSize: " << shmElemSize << std::endl;
          }
        }
        if(connectionType == std::string("remote"))
        {
          std::cout << "\tProtocol: " << protocol << std::endl;
        }
      }
    };


    /**
     * @brief Yaml input kernel port
     * YAML attribute | Details
     * ---------------| ----------------------------
     * port_name      | Registered inport name of kernel class
     * connection_type| "local" or "remote"
     * local_channel  | "raftlib" or ["shm", SHM_ID, SHM_SIZE, SHM_ELEM_SIZE]
     * remote_info    | [protocol, port number] for receive
     */
    class YamlInPort: public YamlPort
    {
      public:
        int bindingPortNum; // remote binding


        YamlInPort()
        {
          bindingPortNum = -1;
        }


        void print()
        {
          printBase();
          std::cout << "InPort Specific -------- " << std::endl;
          if(connectionType == std::string("remote"))
          {
            std::cout << "\tBinding Port Number: " << bindingPortNum << std::endl;
          }
        }
    };


    /**
     * @brief Yaml output kernel port
     * YAML attribute | Details
     * ---------------| ----------------------------
     * port_name      | Outport name
     * connection_type| "local" or "remote"
     * remote_info    | [protocol, ip address, port number] of remote
     * local_channel  | "raftlib" or ["shm", SHM_ID, SHM_SIZE, SHM_ELEM_SIZE]
     * duplicated_from| Registered outport name of kernel class
     */
    class YamlOutPort: public YamlPort
    {
      public:
        std::string semantics; // OutPort semantics is set at runtime by the deployer..

        std::string connectingAddr; // remote connectingAddr
        int connectingPortNum;      // remote connectingPortNum

        std::string duplicatedFrom; // for duplicated ports


        YamlOutPort()
        {
          semantics          = "blocking";

          connectingAddr     = "";
          connectingPortNum  = -1;

          duplicatedFrom     = "";
        }


        void print()
        {
          printBase();
          std::cout << "OutPort Specific -------- " << std::endl;
          if(semantics != "")
            std::cout << "\tSemantics: " << semantics << std::endl;
          if(duplicatedFrom != "")
            std::cout << "\tDuplicated From: " << duplicatedFrom << std::endl;
          if(connectionType == std::string("remote"))
          {
            std::cout << "\tRemote Address: " << connectingAddr << std::endl;
            std::cout << "\tRemote Port Number: " << connectingPortNum << std::endl;
          }
        }
    };
  }
}
#endif

