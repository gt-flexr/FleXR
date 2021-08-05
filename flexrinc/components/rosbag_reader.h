#ifndef __FLEXR_COMPONENT_ROSBAG_READER__
#define __FLEXR_COMPONENT_ROSBAG_READER__

#include <rosbag/bag.h>
#include <rosbag/view.h>

#include "defs.h"
#include "types/types.h"

namespace flexr {
  namespace components {
    class ROSBagReader {
    protected:
      rosbag::Bag bag;
      rosbag::View *view;
      rosbag::View::iterator curMsg;

    public:
      ROSBagReader();
      ROSBagReader(std::string bagFile, std::string topic);
      bool openBag(std::string bagFile, std::string topic);
      void clearSession();
      ~ROSBagReader();
    };
  }
}
#endif

