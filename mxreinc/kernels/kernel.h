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
#include "types/frame.h"
#include "types/kimera/types.h"

namespace mxre
{
  namespace kernels
  {
    class MXREKernel : public raft::kernel
    {
      protected:
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
        MXREKernel(){
          periodMS = 0;
          periodStart = 0;
          periodEnd = 0;
          periodAdj = 0;
#ifdef __PROFILE__
          pid = getpid();
#endif
        }


        /* Destructor */
        ~MXREKernel(){ };


        /* setSleepPeriodMS() */
        void setSleepPeriodMS(int period) { periodMS = period; }


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


        /* duplicateOutPort(): duplicate output port for propagating multi downstream kernels */
        template<typename T>
        void duplicateOutPort(std::string origin, std::string newOut) {
          oPortMap.insert(std::make_pair<std::string, std::string>(origin.c_str(), newOut.c_str()));
          output.addPort<T>(newOut);
        }


        /* sendPrimitiveDuplicate<T>: propagate the primitive-type data into duplicated output ports */
        template<typename T>
        void sendPrimitiveCopy(std::string id, T* data) {
          auto portRange = oPortMap.equal_range(id);
          for(auto i = portRange.first; i != portRange.second; ++i) {
            auto &outData(output[i->second].allocate<T>());
            outData = *data;
            output[i->second].send();
          }
        }


        /* sendCopyFrame: propagate the primitive-type data into duplicated output ports */
        void sendFrameCopy(std::string id, void* data) {
          mxre::types::Frame *frame = (mxre::types::Frame*)data;
          auto portRange = oPortMap.equal_range(id);
          for(auto i = portRange.first; i != portRange.second; ++i) {
            auto &outData(output[i->second].allocate<mxre::types::Frame>());
            outData = frame->clone();
            output[i->second].send();
          }
        }

        void send_imu_cam_type_copy(std::string id, void* data) {
          mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) data;
          auto portRange = oPortMap.equal_range(id);
          for (auto i = portRange.first; i != portRange.second; ++i) {
            auto &outData(output[i->second].allocate<mxre::kimera_type::imu_cam_type>());
            mxre::kimera_type::imu_cam_type data_clone = *cam_data;
            //TODO: release() somewhere
            data_clone.img0 = new cv::Mat(*cam_data->img0);
            data_clone.img1 = new cv::Mat(*cam_data->img1);
          
            outData = data_clone;
            output[i->second].send();
          }
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

