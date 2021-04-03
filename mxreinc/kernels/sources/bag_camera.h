#ifndef __MXRE_BAG_CAMERA__
#define __MXRE_BAG_CAMERA__

#include <bits/stdc++.h>
#include <raft>

#include "defs.h"
#include "types/frame.h"
#include "types/clock_types.h"
#include "components/rosbag_frame_reader.h"
#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {

    class BagCamera: public MXREKernel
    {
    private:
      unsigned int frameIndex;
      int width, height;
      mxre::components::ROSBagFrameReader bagReader;

    public:
      BagCamera(std::string bagPath="", std::string bagTopic="");
      void setFramesToCache(int numFrames, int startFrameIndex);
      ~BagCamera();
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

