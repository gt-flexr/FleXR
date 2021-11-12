#pragma once

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/frame.h"
#include "flexr_core/include/types/types.h"

namespace flexr {
  namespace utils {

    static bool serializeRawFrame(void* msg, uint8_t* &data, uint32_t &size, bool freeMsgData)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(msg);
      uint32_t msgMetaSize = sizeof(*castedFrame);

      size = msgMetaSize + castedFrame->dataSize;

      if(castedFrame->dataSize != castedFrame->data.dataSize ||
         castedFrame->dataSize != castedFrame->data.useAsCVMat().total()*castedFrame->data.useAsCVMat().elemSize())
      {
        debug_print("Message<Frame> dataSize(%d) mismatches to Frame->dataSize (%ld) and real dataSize(%ld)",
                    castedFrame->dataSize, castedFrame->data.dataSize,
                    castedFrame->data.useAsCVMat().total()*castedFrame->data.useAsCVMat().elemSize());
      }

      data = new uint8_t[size];
      memcpy(data,             castedFrame, msgMetaSize);
      memcpy(data+msgMetaSize, castedFrame->data.data, castedFrame->dataSize);

      if(freeMsgData)
      {
        castedFrame->data.release();
      }

      return true;
    }


    static bool serializeEncodedFrame(void* msg, uint8_t* &data, uint32_t &size, bool freeMsgData)
    {
      types::Message<uint8_t*> *castedFrame = static_cast<types::Message<uint8_t*>*>(msg);
      uint32_t metaSize = sizeof(*castedFrame);

      size = metaSize + castedFrame->dataSize;
      data = new uint8_t[size];

      memcpy(data,          castedFrame,       metaSize);
      memcpy(data+metaSize, castedFrame->data, castedFrame->dataSize);

      if(freeMsgData)
      {
        delete castedFrame->data;
        castedFrame->data = nullptr;
      }

      return true;
    }


    template <typename T>
    bool serializeVector(void* msg, uint8_t* &data, uint32_t &size, bool freeMsgData)
    {
      T* castedMsg = static_cast<T*>(msg);
      uint32_t metaSize = sizeof(*castedMsg);
      size = metaSize + castedMsg->dataSize;
      if(castedMsg->dataSize == castedMsg->data.size()*sizeof(castedMsg->data[0]))
      {
        data = new uint8_t[metaSize + castedMsg->dataSize];
        memcpy(data,          castedMsg,       metaSize);
        memcpy(data+metaSize, castedMsg->data.data(), castedMsg->dataSize);
      }
      else
        debug_print("msg->dataSize is not matched to real data size");

      if(freeMsgData)
      {
        castedMsg->data.clear();
      }

      return true;
    }

  }
}

