#ifndef __MXRE_KERNEL__
#define __MXRE_KERNEL__

#include <bits/stdc++.h>
#include <iostream>
#include <raft>
#include <cv.hpp>
#include <raftinc/rafttypes.hpp>

#include "defs.h"
#include "types/clock_types.h"
#include "types/frame.h"

namespace mxre
{
  namespace kernels
  {
    class MXREKernel : public raft::kernel
    {
      protected:
        std::multimap<std::string, std::string> oPortMap;
        template<typename T> void addInputPort(std::string id) { input.addPort<T>(id); }
        template<typename T> void addOutputPort(std::string id) { output.addPort<T>(id); }
#ifdef __PROFILE__
        mxre::types::TimeVal start, end;
#endif

      public:
        /* Constructor */
        MXREKernel(){ }


        /* Destructor */
        ~MXREKernel(){ };


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
    };

  }   // namespace kernels
} // namespace mxre

#endif

