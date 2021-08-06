#ifndef __FLEXR_COMPONENT_ROSBAG_FRAME_READER__
#define __FLEXR_COMPONENT_ROSBAG_FRAME_READER__

#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <sensor_msgs/Image.h>

#include "defs.h"
#include "types/frame.h"
#include "components/rosbag_reader.h"

namespace flexr {
  namespace components {

    /**
     * @brief Component to read a rosbag
     */
    class ROSBagFrameReader : public ROSBagReader {
    private:
      std::vector<flexr::types::Frame> cachedFrames;
      unsigned int frameIndex;

    public:
      ROSBagFrameReader();
      ~ROSBagFrameReader();


      /**
       * @brief Initialize bag frame reader with the topic subscription
       * @param bagFile
       *  Bag file name to open
       * @param topic
       *  Topic name to subscribe
       */
      ROSBagFrameReader(std::string bagFile, std::string topic);


      /**
       * @brief Cache frames to feed the frames without disk IO bottleneck
       * @param numFrames
       *  Number of frames to cache
       */
      void cacheFrames(int numFrames);


      /**
       * @brief Cache frames with skipping to feed the frames without disk IO bottleneck
       * @param numFrames
       *  Number of frames to cache
       * @param skippingFrames
       *  Number of frames to skip from the start
       */
      void cacheFrames(int numFrames, int skippingFrames);


      /**
       * @brief Uncache the cached frames
       */
      void uncacheFrames();


      /**
       * @brief Check the cached
       * @return Boolean of cached or non-cached
       */
      bool isCached();


      /**
       * @brief Print information of cached frames
       */
      void printCachedFrameInfo();


      /**
       * @brief Get the frame from bag file
       * @return FleXR frame
       * @see flexr::types::Frame
       */
      flexr::types::Frame getNextFrameFromBag();


      /**
       * @brief Get the frame from cached frames
       * @return FleXR frame
       * @see flexr::types::Frame
       */
      flexr::types::Frame getNextFrameFromCachedFrames();


      /**
       * @brief Get the frame
       * @details If the frame reader caches frames, a frame from the cached frames is returned. Otherwise, a frame is
       *  returned from the bag file.
       * @return FleXR frame
       * @see flexr::types::Frame
       */
      flexr::types::Frame getNextFrame();
    };

  }
}

#endif

