#ifndef __MXRE_COMPONENT_ROSBAG_FRAME_READER__
#define __MXRE_COMPONENT_ROSBAG_FRAME_READER__

#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <sensor_msgs/Image.h>

#include "defs.h"
#include "types/frame.h"
#include "components/rosbag_reader.h"

namespace mxre {
  namespace components {
    class ROSBagFrameReader : public ROSBagReader {
    private:
      std::vector<mxre::types::Frame> cachedFrames;
      unsigned int frameIndex;
    public:
      ROSBagFrameReader();
      ROSBagFrameReader(std::string bagFile, std::string topic);
      ~ROSBagFrameReader();

      void cacheFrames(int numFrames);
      void cacheFrames(int numFrames, int skippingFrames);
      void uncacheFrames();
      bool isCached();

      void printCachedFrameInfo();
      mxre::types::Frame getNextFrameFromBag();
      mxre::types::Frame getNextFrameFromCachedFrames();
      mxre::types::Frame getNextFrame();
    };
  }
}
#endif

