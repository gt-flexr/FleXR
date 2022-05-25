#ifndef __FLEXR_CORE_KERNEL__
#define __FLEXR_CORE_KERNEL__

#include <bits/stdc++.h>
#include <iostream>
#include <raft>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <raftinc/rafttypes.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <unistd.h>

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/types.h"
#include "flexr_core/include/types/frame.h"

// Common Components
#include "flexr_core/include/components/flexr_port_manager.h"
#include "flexr_core/include/components/logger.h"
#include "flexr_core/include/components/frequency_manager.h"

#include "flexr_core/include/yaml_ports.h"

#include "flexr_core/include/utils/local_copy_functions.h"
#include "flexr_core/include/utils/serialize_functions.h"
#include "flexr_core/include/utils/deserialize_functions.h"

namespace flexr
{
  namespace kernels
  {

    /**
     * @brief Base kernel
     */
    class FleXRKernel : public raft::kernel
    {
      protected:
        std::string name, id;
        components::FleXRPortManager portManager;
        components::Logger logger;
        components::FrequencyManager freqManager;

      public:
        FleXRKernel(): portManager(&input, &output)
        {
          id = "no_id";
          name = "FleXRKernel";
        }


        ~FleXRKernel()
        {};


        FleXRKernel(std::string id): FleXRKernel()
        {
          this->id = id;
        }


        std::string getId()
        {
          return id;
        }


        void setName(std::string newName)
        {
          name = newName;
        }


        std::string getName()
        {
          return name;
        }


        virtual raft::kstatus run() { return raft::stop; }


        template <typename T>
        void activateInPortAsLocal(yaml::YamlInPort inPort)
        {
          portManager.activateInPortAsLocal<T>(inPort);
        }


        template <typename T>
        void activateInPortAsRemote(yaml::YamlInPort inPort)
        {
          portManager.activateInPortAsRemote<T>(inPort);
        }


        template <typename T>
        void activateOutPortAsLocal(yaml::YamlOutPort outPort)
        {
          portManager.activateOutPortAsLocal<T>(outPort);
        }


        template <typename T>
        void activateOutPortAsRemote(yaml::YamlOutPort outPort)
        {
          portManager.activateOutPortAsRemote(outPort);
        }


        template <typename T>
        void duplicateOutPortAsLocal(yaml::YamlOutPort outPort)
        {
          portManager.duplicateOutPortAsLocal<T>(outPort);
        }


        template <typename T>
        void duplicateOutPortAsRemote(yaml::YamlOutPort outPort)
        {
          portManager.duplicateOutPortAsRemote<T>(outPort);
        }


        void setLogger(std::string loggerID, std::string logFileName)
        {
          logger.set(loggerID, logFileName);
        }


        void setFrequency(int targetFrequency)
        {
          freqManager.setFrequency(targetFrequency);
        }
    };


    static void runPipeline(raft::map *pipeline) {
      pipeline->exe();
    }


    static void runSingleKernel (FleXRKernel *kernel)
    {
      while (1) {
        raft::kstatus ret = kernel->run();
        if (ret == raft::stop) break;
      }
    }

  }   // namespace kernels
} // namespace flexr

#endif

