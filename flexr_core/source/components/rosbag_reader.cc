#ifdef __FLEXR_USE_ROSBAG__

#include <bits/stdc++.h>
#include <flexr_core/include/components/rosbag_reader.h>

namespace flexr {
  namespace components {
    ROSBagReader::ROSBagReader()
    {
      view = nullptr;
    }


    ROSBagReader::ROSBagReader(std::string bagFile, std::string topic)
    {
      view = nullptr;
      openBag(bagFile, topic);
    }


    ROSBagReader::~ROSBagReader()
    {
      clearSession();
    }


    bool ROSBagReader::openBag(std::string bagFile, std::string topic) {
      clearSession();

      bag.open(bagFile, rosbag::bagmode::Read);
      if(bag.isOpen() == false) {
        debug_print("Bag file is failed to open.");
        return false;
      }

      view = new rosbag::View(bag, rosbag::TopicQuery(topic));
      curMsg = view->begin();

      return true;
    }


    void ROSBagReader::clearSession()
    {
      if(view) {
        debug_print("clear rosbag::View");
        delete view; view = nullptr;
      }
      if(bag.isOpen()) {
        debug_print("clear rosbag::Bag");
        bag.close();
      }
    }
  }
}

#endif
