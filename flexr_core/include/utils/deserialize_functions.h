#pragma once

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/types.h"
#include "flexr_core/include/types/frame.h"

namespace flexr {
  namespace utils {

    static bool deserializeRawFrame(uint8_t* &data, uint32_t &size, void** msg)
    {
      bool received = true;
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(*msg);
      uint32_t msgMetaSize   = sizeof(*castedFrame);

      memcpy(castedFrame, data, msgMetaSize);

      if(size-msgMetaSize != castedFrame->dataSize || size-msgMetaSize != castedFrame->data.dataSize)
      {
        debug_print("Data size(%d) mismatches to %d, %d", size, msgMetaSize, castedFrame->dataSize);
        received = false;
      }
      else
      {
        castedFrame->data.data = new uint8_t[castedFrame->dataSize];
        memcpy(castedFrame->data.data, data+msgMetaSize, castedFrame->dataSize);
      }

      delete data;
      data = nullptr;
      return received;
    }


    static bool deserializeEncodedFrame(uint8_t* &data, uint32_t &size, void** msg)
    {
      bool received = true;
      types::Message<uint8_t*> *castedFrame = static_cast<types::Message<uint8_t*>*>(*msg);
      uint32_t metaSize = sizeof(*castedFrame);

      memcpy(castedFrame, data, metaSize);
      if(size-metaSize == castedFrame->dataSize)
      {
        castedFrame->data = new uint8_t[castedFrame->dataSize];
        memcpy(castedFrame->data, data+metaSize, castedFrame->dataSize);
      }
      else
      {
        debug_print("received castedFrame->dataSize is not matched to size-metaSize: %d / %d",
                    castedFrame->dataSize, size-metaSize);
        received = false;
      }

      delete data;
      data = nullptr;
      return received;
    }


    template <typename T>
    bool deserializeVector(uint8_t* &data, uint32_t &size, void** msg)
    {
      bool received = true;
      T* castedMsg = static_cast<T*>(*msg);
      uint32_t metaSize = sizeof(*castedMsg);

      memcpy(castedMsg, data, metaSize);
      if(size-metaSize == castedMsg->dataSize)
      {
        uint32_t elemSize = sizeof(decltype(castedMsg->data.back()));
        uint32_t numElem  = (castedMsg->dataSize) / elemSize;

        castedMsg->data.resize(numElem);
        memcpy((void*)castedMsg->data.data(), data+metaSize, castedMsg->dataSize);
      }
      else
      {
        debug_print("received castedMsg->dataSize is not matched to size-metaSize: %d / %d",
                    castedMsg->dataSize, size-metaSize);
        received = false;

      }

      delete data;
      data = nullptr;
      return received;
    }

  }
}

