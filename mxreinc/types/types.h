#ifndef __MXRE_TYPES__
#define __MXRE_TYPES__

#include <bits/stdc++.h>
#include <defs.h>
#include <types/frame.h>

// TODO: integrate all the types into this file
namespace mxre {
  namespace types {
    typedef struct RTPTrackingInfo {
      int32_t invoice; // for sanity check
      uint32_t dataSize;
      char tag[MXRE_MSG_TAG_SIZE];
      uint32_t seq;
      double ts;
    } RTPTrackingInfo;


    template <class T>
    class Message
    {
      public:
      char     tag[MXRE_MSG_TAG_SIZE];
      uint32_t seq;
      double   ts;
      uint32_t dataSize;
      T        data;
    };


    class ObjectPose
    {
      public:
      float rx, ry, rz;
      float tx, ty, tz;
    };


    template <typename T>
    void freePrimitiveMsg(void *msg)
    {
      T* castedMessage = static_cast<T*>(msg);
      delete castedMessage;
    }


    static void freeFrameMsg(void *msg)
    {
      Message<Frame> *castedFrame = static_cast<Message<Frame>*>(msg);
      castedFrame->data.release();
      delete castedFrame;
    }


    static void allocFrameWithBuffer(void **msg, int size)
    {
      Message<Frame> *castedFrame = new Message<Frame>();
      castedFrame->data.data     = new unsigned char[size];
      castedFrame->data.dataSize = size;
      *msg = castedFrame;
    }
  }
}
#endif

