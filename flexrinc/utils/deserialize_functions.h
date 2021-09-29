#ifndef __FLEXR_UTILS_DESERIALIZE_FUNCS__
#define __FLEXR_UTILS_DESERIALIZE_FUNCS__

#include "defs.h"
#include "types/cv/types.h"
#include "types/types.h"
#include "types/frame.h"
#include "components/flexr_port.h"

namespace flexr {
  namespace utils {
    static void deserializeDecodedFrame(uint8_t* &data, uint32_t &size, void** msg)
    {
      types::Message<uint8_t*> *castedFrame = static_cast<types::Message<uint8_t*>*>(*msg);
      uint32_t metaSize = sizeof(castedFrame);

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

