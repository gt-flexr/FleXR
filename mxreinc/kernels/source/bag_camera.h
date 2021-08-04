#ifndef __MXRE_BAG_CAMERA__
#define __MXRE_BAG_CAMERA__

#include <bits/stdc++.h>
#include <raft>

#include "defs.h"
#include "types/frame.h"
#include "types/clock_types.h"
#include "components/rosbag_frame_reader.h"
#include "components/frequency_manager.h"
#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {
    using BagCameraMsgType = types::Message<types::Frame>;

    class BagCamera: public MXREKernel
    {
    private:
      uint32_t seq;
      components::ROSBagFrameReader bagReader;
      components::FrequencyManager freqManager;

    public:
      BagCamera(std::string tag="", std::string bagPath="", std::string bagTopic="", int targetFps=30);
      void setFramesToCache(int numFrames, int startFrameIndex);
      ~BagCamera();
      void activateOutPortAsRemote(const std::string tag, const std::string addr, int portNumber)
      {
        debug_print("not allow remote port activation.");
      }
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

