#include <components/rosbag_reader.h>
#include <bits/stdc++.h>

namespace mxre {
  namespace components {
    ROSBagReader::ROSBagReader()
    {
      view = nullptr;
    }


    ROSBagReader::ROSBagReader(std::string bagFile, std::string topic)
    {
      openBag(bagFile, topic);
      view = nullptr;
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
      if(view != nullptr) {
        delete view; view = nullptr;
      }
      if(bag.isOpen()) bag.close();
    }
  }
}

