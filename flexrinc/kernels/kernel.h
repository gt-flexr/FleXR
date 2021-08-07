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

    /**
     * @brief Base kernel
     */
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


        ~FleXRKernel()
        {};


        FleXRKernel(std::string id): FleXRKernel()
        {
          this->id = id;
        }


        /**
         * @brief Kernel interface for run
         */
        virtual raft::kstatus run() { return raft::stop; }


        /**
         * @brief Kernel interface to activate input port as local
         * @param tag
         *  Port tag to activate
         * @see flexr::components::FleXRPortManager.activateInPortAsLocal
         */
        template <typename T>
        void activateInPortAsLocal(const std::string tag)
        {
          portManager.activateInPortAsLocal<T>(tag);
        }


        /**
         * @brief Kernel interface to activate input port as remote
         * @param tag
         *  Tag of activating port
         * @param portNumber
         *  Port number to activate
         * @see flexr::components::FleXRPortManager.activateInPortAsRemote
         */
        template <typename T>
        void activateInPortAsRemote(const std::string tag, int portNumber)
        {
          portManager.activateInPortAsRemote<T>(tag, portNumber);
        }


        /**
         * @brief Kernel interface to activate output port as local
         * @param tag
         *  Tag of activating port
         * @see flexr::components::FleXRPortManager.activateOutPortAsLocal
         */
        template <typename T>
        void activateOutPortAsLocal(const std::string tag)
        {
          portManager.activateOutPortAsLocal<T>(tag);
        }


        /**
         * @brief Kernel interface to activate output port as remote
         * @param tag
         *  Tag of activating port
         * @param addr
         *  Address of remote node to connect
         * @param portNumber
         *  Port number of remote node to connect
         * @see flexr::components::FleXRPortManager.activateOutPortAsRemote
         */
        template <typename T>
        void activateOutPortAsRemote(const std::string tag, const std::string addr, int portNumber)
        {
          portManager.activateOutPortAsRemote<T>(tag, addr, portNumber);
        }


        /**
         * @brief Kernel interface to duplicate an activated output port to a local output port
         * @param originTag
         *  Tag of the original port to duplicate
         * @param newTag
         *  Tag of a new port
         * @see flexr::components::FleXRPortManager.duplicateOutPortAsLocal
         */
        template <typename T>
        void duplicateOutPortAsLocal(const std::string originTag, const std::string newTag)
        {
          portManager.duplicateOutPortAsLocal<T>(originTag, newTag);
        }


        /**
         * @brief Kernel interface to duplicate an activated output port to a remote output port
         * @param originTag
         *  Tag of activating port
         * @param newTag
         *  Tag of a new port
         * @param addr
         *  Address of remote node to connect a new port
         * @param portNumber
         *  Port number of remote node to connect a new port
         * @see flexr::components::FleXRPortManager.duplicateOutPortAsRemote
         */
        template <typename T>
        void duplicateOutPortAsRemote(const std::string originTag, const std::string newTag,
                                      const std::string addr, int portNumber)
        {
          portManager.duplicateOutPortAsRemote<T>(originTag, newTag, addr, portNumber);
        }


        /**
         * @brief Kernel interface to set a logger
         * @param loggerID
         *  Logger identifier
         * @param logFileName
         *  File name of the saved log
         * @see flexr::components::Logger
         */
        void setLogger(std::string loggerID, std::string logFileName)
        {
          logger.set(loggerID, logFileName);
        }


       void setDebugMode()
        {
          debugMode = true;
        }
    };


    /**
     * @brief Thread function for running a pipeline
     * @details std::thread t1(runPipeline, &pipeline);
     * @param pipeline
     *  Pointer to the pipeline to run
     */
    static void runPipeline(raft::map *pipeline) {
      pipeline->exe();
    }


    /**
     * @brief Thread function for running a single kernel without pipelining
     * @details std::thread t1(runSigleKernel, &kernel);
     * @param kernel
     *  Pointer to the kernel to run
     */
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

