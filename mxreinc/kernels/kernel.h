#ifndef __MXRE_KERNEL__
#define __MXRE_KERNEL__

#include <bits/stdc++.h>
#include <iostream>
#include <raft>
#include <cv.hpp>
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

namespace mxre
{
  namespace kernels
  {
    // To run multiple pipelines
    // std::thread T1(runPipeline, &pipeline);
    // T1.join();
    static void runPipeline(raft::map *pipeline) {
      pipeline->exe();
    }

    // the current raftlib does not support single kernel run. When a single kernel is offloaded, there is no way to
    // run it.
    //   std::thread T1(runSigleKernel, &kernel);
    //   T1.join();
    static void runSingleKernel (raft::kernel *kernel)
    {
      while (1) {
        raft::kstatus ret = kernel->run();
        if (ret == raft::stop) break;
      }
    }

    class MXREKernel : public raft::kernel
    {
      protected:
        std::string id;
        unsigned int periodMS;
        unsigned int periodStart, periodEnd, periodAdj;
        std::multimap<std::string, std::string> oPortMap;
        template<typename T> void addInputPort(std::string id) { input.addPort<T>(id); }
        template<typename T> void addOutputPort(std::string id) { output.addPort<T>(id); }
        void sleepForMS(int period) {
          if(period > 0) std::this_thread::sleep_for(std::chrono::milliseconds(period));
        }


#ifdef __PROFILE__
        mxre::types::TimeVal start, end;
        double startTimeStamp, endTimeStamp;
        std::shared_ptr<spdlog::logger> logger;
        int pid;
#endif

      public:
        /* Constructor */
        MXREKernel()
        {
          periodMS = 0;
          periodStart = 0;
          periodEnd = 0;
          periodAdj = 0;
          id = "no_id";
          pid = getpid();
        }

        MXREKernel(std::string id): MXREKernel()
        {
          this->id = id;
        }


        /* Destructor */
        ~MXREKernel(){ };


        /* setSleepPeriodMS() */
        void setSleepPeriodMS(int period) { periodMS = period; }


        /* setFPS() */
        void setFPS(unsigned int fps) { setSleepPeriodMS((int)(1000/fps)); }


        /* run(): set in/out ports and call logic() */
        virtual raft::kstatus run(){ return raft::proceed; }


        /* logic(): run kernel logic */
        bool logic(){ return true; }


        /* recyclePort(): recycle input port */
        void recyclePort(std::string id) { input[id].recycle(); }


        /* checkPort(): check the input port for non-blocking input port */
        bool checkPort(std::string id) {
          auto &port(input[id]);
          if(port.size() > 0) return true;
          return false;
        }


        /* duplicateOutPort<T>(origin, newOut): duplicate output port for propagating multi downstream kernels */
        template<typename T>
        void duplicateOutPort(std::string origin, std::string newOut) {
          oPortMap.insert(std::make_pair<std::string, std::string>(origin.c_str(), newOut.c_str()));
          output.addPort<T>(newOut);
        }


        /* sendPrimitiveCopy<T>(id, data): propagate the primitive-type data into duplicated output ports */
        template<typename T>
        void sendPrimitiveCopy(std::string id, T& data) {
          auto portRange = oPortMap.equal_range(id);
          for(auto i = portRange.first; i != portRange.second; ++i) {
            T &outData = output[i->second].allocate<T>();
            outData = data;
            output[i->second].send();
          }
        }


        /* sendFrames: propagate the primitive-type data into duplicated output ports */
        void sendFrames(std::string id, types::Message<types::Frame> &frame) {
          auto portRange = oPortMap.equal_range(id);
          for(auto i = portRange.first; i != portRange.second; ++i) {
            types::Message<types::Frame> &copiedFrame = output[i->second].allocate<types::Message<types::Frame>>();
            strcpy(copiedFrame.tag, frame.tag);
            copiedFrame.seq = frame.seq;
            copiedFrame.ts = frame.ts;
            copiedFrame.data = frame.data.clone();
            output[i->second].send();
          }
          output[id].send();
        }

#ifdef __PROFILE__
        void initLoggerST(std::string loggerName, std::string fileName) {
          logger = spdlog::basic_logger_st(loggerName, fileName);
        }

        void initLoggerMT(std::string loggerName, std::string fileName) {
          logger = spdlog::basic_logger_mt(loggerName, fileName);
        }
#endif

    };

  }   // namespace kernels
} // namespace mxre

#endif

