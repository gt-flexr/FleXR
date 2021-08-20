#ifndef __FLEXR_YAML_LOCAL_CONNECTION__
#define __FLEXR_YAML_LOCAL_CONNECTION__

#include <bits/stdc++.h>
#include "defs.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml local lonnection
     */
    class YamlLocalConnection
    {
      public:
        std::string sendKernel, sendPortName;
        std::string recvKernel, recvPortName;
        int queueSize;

        /**
         * @brief Print connection info
         */
        void printConnection()
        {
          std::cout << "Connection --------" << std::endl;
          std::cout << "\tSending kernel: " << sendKernel << ", " << sendPortName << std::endl;
          std::cout << "\tReceiving kernel: " << recvKernel << ", " << recvPortName << std::endl;
          std::cout << "\tQueue Size: " << queueSize << std::endl;
        }
    };

  }
}
#endif

