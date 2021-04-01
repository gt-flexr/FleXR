#include <components/rosbag_frame_reader.h>
#include <bits/stdc++.h>

namespace mxre {
  namespace components {
    ROSBagFrameReader::ROSBagFrameReader(std::string bagFile, std::string topic): ROSBagReader(bagFile, topic) {
      frameIndex = 0;
    }


    ROSBagFrameReader::~ROSBagFrameReader() {
      for(unsigned int i = 0; i < cachedFrames.size(); i++) cachedFrames[i].release();
    }

    void ROSBagFrameReader::cacheFrames(int numFrames)
    {
      for(int i = 0; i < numFrames; i++) {
        sensor_msgs::Image::ConstPtr image = curMsg->instantiate<sensor_msgs::Image>();
        mxre::types::Frame frame(image->height, image->width, CV_8UC3, -1, -1);
        if(frame.dataSize == image->data.size()) {
          memcpy(frame.data, image->data.data(), image->data.size());
          cachedFrames.push_back(frame);
          curMsg++;
        }
        else {
          debug_print("allocated memory for mxre::type:Frame is not equal to the read data");
          exit(1);
        }
      }
      printCachedFrameInfo();
    }


    void ROSBagFrameReader::printCachedFrameInfo()
    {
      debug_print("Num of Cached Frames: %ld" , cachedFrames.size());
      for(std::vector<mxre::types::Frame>::iterator iter = cachedFrames.begin(); iter != cachedFrames.end(); iter++) {
        debug_print("\tEach frame Info");
        debug_print("\tWidth(%ld) Height(%ld) Size(%ld) ElemSize(%ld) TotalElem(%ld)", iter->cols, iter->rows,
                    iter->dataSize, iter->elemSize, iter->totalElem);
      }
    }


    mxre::types::Frame ROSBagFrameReader::getNextFrameFromBag()
    {
      sensor_msgs::Image::ConstPtr image = curMsg->instantiate<sensor_msgs::Image>();
      mxre::types::Frame frame(image->height, image->width, CV_8UC3, -1, -1);
      if(frame.dataSize == image->data.size()) {
        memcpy(frame.data, image->data.data(), image->data.size());
        cachedFrames.push_back(frame);
        curMsg++;
      }
      else {
        debug_print("allocated memory for mxre::type:Frame is not equal to the read data");
        exit(1);
      }

      return frame;
    }


    mxre::types::Frame ROSBagFrameReader::getNextFrameFromCachedFrames()
    {
      mxre::types::Frame retFrame(cachedFrames[frameIndex].useAsCVMat(), -1, -1);
      frameIndex = (frameIndex + 1) % cachedFrames.size();
      return retFrame;
    }
  }
}

