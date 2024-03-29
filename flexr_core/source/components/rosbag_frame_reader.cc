#ifdef __FLEXR_USE_ROSBAG__

#include <bits/stdc++.h>
#include <flexr_core/include/components/rosbag_reader.h>
#include <flexr_core/include/components/rosbag_frame_reader.h>

namespace flexr {
  namespace components {
    ROSBagFrameReader::ROSBagFrameReader(): ROSBagReader()
    {
      frameIndex = 0;
      cachedFrames.clear();
    }


    ROSBagFrameReader::ROSBagFrameReader(std::string bagFile, std::string topic): ROSBagReader(bagFile, topic)
    {
      frameIndex = 0;
    }


    ROSBagFrameReader::~ROSBagFrameReader() {
      uncacheFrames();
    }


    void ROSBagFrameReader::cacheFrames(int numFrames)
    {
      if(bag.isOpen() == false) {
        debug_print("bag file is not open");
        return;
      }

      for(int i = 0; i < numFrames; i++) {
        sensor_msgs::Image::ConstPtr image = curMsg->instantiate<sensor_msgs::Image>();
        if(image == NULL) debug_print("failed to read bag with frame %dth", i);
        else {
          flexr::types::Frame frame(image->height, image->width, CV_8UC3);
          if(frame.dataSize == image->data.size()) {
            memcpy(frame.data, image->data.data(), image->data.size());
            cachedFrames.push_back(frame);
            curMsg++;
          }
          else {
            debug_print("allocated memory for flexr::type:Frame is not equal to the read data");
            exit(1);
          }
        }
      }
    }


    void ROSBagFrameReader::cacheFrames(int numFrames, int skippingFrames)
    {
      if(bag.isOpen() == false) {
        debug_print("bag file is not open");
        return;
      }

      for(int i = 0; i < skippingFrames; i++) curMsg++;
      debug_print("start to read a bag file from %dth to %dth frames", skippingFrames, skippingFrames + numFrames);
      cacheFrames(numFrames);
    }


    void ROSBagFrameReader::uncacheFrames()
    {
      if(isCached()) for(unsigned int i = 0; i < cachedFrames.size(); i++) cachedFrames[i].release();
      cachedFrames.clear();
    }


    bool ROSBagFrameReader::isCached()
    {
      if(cachedFrames.size() > 0) return true;
      return false;
    }


    void ROSBagFrameReader::printCachedFrameInfo()
    {
      debug_print("Num of Cached Frames: %ld" , cachedFrames.size());
      int i = 0;
      for(std::vector<flexr::types::Frame>::iterator iter = cachedFrames.begin(); iter != cachedFrames.end(); iter++) {
        debug_print("\t%d Width(%ld) Height(%ld) Size(%ld) ElemSize(%ld) TotalElem(%ld)", i, iter->cols, iter->rows,
                    iter->dataSize, iter->elemSize, iter->totalElem);
        i++;
      }
    }


    flexr::types::Frame ROSBagFrameReader::getNextFrameFromBag()
    {
      sensor_msgs::Image::ConstPtr image = curMsg->instantiate<sensor_msgs::Image>();
      flexr::types::Frame frame(image->height, image->width, CV_8UC3);
      if(frame.dataSize == image->data.size()) {
        memcpy(frame.data, image->data.data(), image->data.size());
        curMsg++;
      }
      else {
        debug_print("allocated memory for flexr::type:Frame is not equal to the read data");
        exit(1);
      }

      return frame;
    }


    flexr::types::Frame ROSBagFrameReader::getNextFrameFromCachedFrames()
    {
      flexr::types::Frame retFrame(cachedFrames[frameIndex].useAsCVMat());
      frameIndex = (frameIndex + 1) % cachedFrames.size();
      return retFrame;
    }


    flexr::types::Frame ROSBagFrameReader::getNextFrame()
    {
      if(isCached()) return getNextFrameFromCachedFrames();
      else return getNextFrameFromBag();
    }
  }
}

#endif

