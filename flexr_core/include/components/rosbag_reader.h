#pragma once

#ifdef __FLEXR_USE_ROSBAG__

#include <rosbag/bag.h>
#include <rosbag/view.h>

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/types.h"

namespace flexr {
  namespace components {

    /**
     * @brief Component to read recorded bag files
     */
    class ROSBagReader {
    protected:
      rosbag::Bag bag;
      rosbag::View *view;
      rosbag::View::iterator curMsg;

    public:
      ROSBagReader();


      /**
       * @brief Initialize bag viewer with the topic subscription
       * @param bagFile
       *  Bag file name to open
       * @param topic
       *  Topic name to subscribe
       */
      ROSBagReader(std::string bagFile, std::string topic);


      /**
       * @brief Open a bag file and set bag viewer with the topic subscription
       * @param bagFile
       *  Bag file name to open
       * @param topic
       *  Topic name to subscribe
       */
      bool openBag(std::string bagFile, std::string topic);


      void clearSession();
      ~ROSBagReader();
    };
  }
}

#endif

