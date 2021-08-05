#ifndef __FLEXR_KERNEL__
#define __FLEXR_KERNEL__

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

#include "defs.h"
#include "types/clock_types.h"
#include "types/types.h"
#include "types/frame.h"

// Common Components
#include "components/flexr_port_manager.h"
#include "components/logger.h"

namespace flexr
{
  namespace kernels
  {
    // Thread function for running multiple pipelines
    //   std::thread T1(runPipeline, &pipeline);
    //   T1.join();
    static void runPipeline(raft::map *pipeline) {
      pipeline->exe();
    }

    // Thread function for running a single kernel without pipelining
    //   std::thread T1(runSigleKernel, &kernel);
    //   T1.join();
    static void runSingleKernel (raft::kernel *kernel)
    {
      while (1) {
        raft::kstatus ret = kernel->run();
        if (ret == raft::stop) break;
      }
    }


    class FleXRKernel : public raft::kernel
    {
      protected:
        std::string id;
        components::FleXRPortManager portManager;
        components::Logger logger;
        bool debugMode;

      public:
        FleXRKernel(): portManager(&input, &output)
        {
          id = "no_id";
          debugMode = false;
        }

        FleXRKernel(std::string id): FleXRKernel()
        {
          this->id = id;
        }

        ~FleXRKernel()
        {};

        virtual raft::kstatus run() { return raft::stop; }

        /*
         * Interfaces for Kernel Deployers
         *   Port Activation
         *   - activateInPortAsLocal
         *   - activateInPortAsRemote
         *   - activateOutPortAsLocal
         *   - activateOutPortAsRemote
         *   - duplicateOutPortAsLocal
         *   - duplicateOutPortAsRemote
         *   Logger Setting
         *   - setLogger
         */
        template <typename T>
        void activateInPortAsLocal(const std::string tag)
        {
          portManager.activateInPortAsLocal<T>(tag);
        }

        template <typename T>
        void activateInPortAsRemote(const std::string tag, int portNumber)
        {
          portManager.activateInPortAsRemote<T>(tag, portNumber);
        }

        template <typename T>
        void activateOutPortAsLocal(const std::string tag)
        {
          portManager.activateOutPortAsLocal<T>(tag);
        }

        template <typename T>
        void activateOutPortAsRemote(const std::string tag, const std::string addr, int portNumber)
        {
          portManager.activateOutPortAsRemote<T>(tag, addr, portNumber);
        }

        template <typename T>
        void duplicateOutPortAsLocal(const std::string originTag, const std::string newTag)
        {
          portManager.duplicateOutPortAsLocal<T>(originTag, newTag);
        }

        template <typename T>
        void duplicateOutPortAsRemote(const std::string originTag, const std::string newTag,
                                      const std::string addr, int portNumber)
        {
          portManager.duplicateOutPortAsRemote<T>(originTag, newTag, addr, portNumber);
        }

        void setLogger(std::string loggerID, std::string logFileName)
        {
          logger.set(loggerID, logFileName);
        }

        void setDebugMode()
        {
          debugMode = true;
        }
    };

  }   // namespace kernels
} // namespace flexr

#endif

