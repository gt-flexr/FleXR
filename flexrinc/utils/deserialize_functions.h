#ifndef __FLEXR_UTILS_DESERIALIZE_FUNCS__
#define __FLEXR_UTILS_DESERIALIZE_FUNCS__

#include "defs.h"
#include "types/types.h"
#include "types/frame.h"
#include "components/flexr_port.h"

namespace flexr {
  namespace utils {

    static void deserializeRawFrame(uint8_t* &data, uint32_t &size, void** msg)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(*msg);
      uint32_t msgMetaSize   = sizeof(*castedFrame);
      uint32_t frameMetaSize = sizeof(types::Frame);

      uint8_t *temp = data;
      memcpy(castedFrame,        temp, msgMetaSize);    temp += msgMetaSize;
      memcpy(&castedFrame->data, temp, frameMetaSize);  temp += frameMetaSize;

      if(size-msgMetaSize-frameMetaSize != castedFrame->dataSize &&
         size-msgMetaSize-frameMetaSize != castedFrame->data.dataSize)
      {
        debug_print("Data size(%d) mismatches to %d, %d, %d", size, msgMetaSize, frameMetaSize, castedFrame->dataSize);
      }

      castedFrame->data.data = new uint8_t[castedFrame->dataSize];
      memcpy(castedFrame->data.data, temp, castedFrame->dataSize);

      delete data;
      data = nullptr;
    }


    static void deserializeEncodedFrame(uint8_t* &data, uint32_t &size, void** msg)
    {
      types::Message<uint8_t*> *castedFrame = static_cast<types::Message<uint8_t*>*>(*msg);
      uint32_t metaSize = sizeof(*castedFrame);

      memcpy(castedFrame, data, metaSize);
      if(size-metaSize == castedFrame->dataSize)
      {
        castedFrame->data = new uint8_t[castedFrame->dataSize];
        memcpy(castedFrame->data, data+metaSize, castedFrame->dataSize);
      }
      else
        debug_print("received castedFrame->dataSize is not matched to size-metaSize");

      delete data;
      data = nullptr;
    }


    template <typename T>
    void deserializeVector(uint8_t* &data, uint32_t &size, void** msg)
    {
      T* castedMsg = static_cast<T*>(*msg);
      uint32_t metaSize = sizeof(castedMsg);

      memcpy(castedMsg, data, metaSize);
      if(size-metaSize == castedMsg->dataSize)
      {
        uint32_t elemSize = sizeof(decltype(castedMsg->data.back()));
        uint32_t numElem  = (castedMsg->dataSize) / elemSize;

        castedMsg->data.resize(numElem);
        memcpy(castedMsg->data, data+metaSize, castedMsg->dataSize);
      }
      else
        debug_print("received castedFrame->dataSize is not matched to size-metaSize");

      delete data;
      data = nullptr;
    }

  }
}

#endif

