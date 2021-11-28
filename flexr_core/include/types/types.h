#ifndef __FLEXR_CORE_TYPES__
#define __FLEXR_CORE_TYPES__

#include <bits/stdc++.h>
#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/frame.h"

// TODO: integrate all the types into this file
namespace flexr {
  namespace types {
    template <class T>
    class Message
    {
      public:
      char     tag[FLEXR_MSG_TAG_SIZE];
      uint32_t seq;
      uint32_t ts;
      uint32_t dataSize;
      T        data;

      void printHeader()
      {
        printf("Message Header: %s \n", tag);
        printf("\tseq: %d, ts: %ld, dataSize %d\n", seq, ts, dataSize);
      }

      void setHeader(std::string tag, uint32_t seq, uint32_t ts, uint32_t dataSize)
      {
        std::strcpy(this->tag, tag.c_str());
        this->seq      = seq;
        this->ts       = ts;
        this->dataSize = dataSize;
      }
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

