#ifndef __FLEXR_BAG_CAMERA__
#define __FLEXR_BAG_CAMERA__

#include <bits/stdc++.h>
#include <raft>

#include "defs.h"
#include "types/frame.h"
#include "types/clock_types.h"
#include "components/rosbag_frame_reader.h"
#include "components/frequency_manager.h"
#include "kernels/kernel.h"

namespace flexr
{
  namespace kernels
  {
    using BagCameraMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to get camera frames from a bag file
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * out_frame      | @ref flexr::types::Message< @ref flexr::types::Frame>
     */
    class BagCamera: public FleXRKernel
    {
    private:
      uint32_t seq;
      components::ROSBagFrameReader bagReader;
      components::FrequencyManager freqManager;

    public:
      ~BagCamera();


      /**
       * @brief Initialize bag camera kernel
       * @param id
       *  Kernel ID
       * @param bagPath
       *  Bag file path to read
       * @param bagTopic
       *  Bag topic to subscribe
       * @param targetFps
       *  Target frequency to feed frames
       * @see flexr::components::ROSBagFrameReader
       */
      BagCamera(std::string id="", std::string bagPath="", std::string bagTopic="", int targetFps=30);


      /**
       * @brief Set frame caching
       * @param numFrames
       *  Number of frames to cache
       * @param startFrameIndex
       *  Number of frames to skip from the start
       * @see flexr::components::ROSBagFrameReader
       */
      void setFramesToCache(int numFrames, int startFrameIndex);


      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace flexr

#endif

