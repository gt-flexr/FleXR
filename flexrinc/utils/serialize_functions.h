#ifndef __FLEXR_UTILS_SERIALIZE_FUNCS__
#define __FLEXR_UTILS_SERIALIZE_FUNCS__

#include <zmq.hpp>
#include "defs.h"
#include "types/cv/types.h"
#include "types/frame.h"
#include "types/types.h"
#include "components/flexr_port.h"

namespace flexr {
  namespace utils {

    static void serializeEncodedFrame(void* msg, uint8_t* &data, uint32_t &size)
    {
      types::Message<uint8_t*> *castedFrame = static_cast<types::Message<uint8_t*>*>(msg);
      uint32_t metaSize = sizeof(castedFrame);
      data = new uint8_t[metaSize + castedFrame->dataSize];
      memcpy(data,          castedFrame, metaSize);
      memcpy(data+metaSize, castedFrame->data, castedFrame->dataSize);

      delete castedFrame->data;
      castedFrame->data = nullptr;
    }


    template <typename T>
    void serializeVector(void* msg, uint8_t* &data, uint32_t &size)
    {
      T* castedMsg = static_cast<T*>(msg);
      uint32_t metaSize = sizeof(castedMsg);
      if(castedMsg->dataSize == castedMsg->data.size()*sizeof(castedMsg->data[0]))
      {
        data = new uint8_t[metaSize + castedMsg->dataSize];
        memcpy(data,          castedMsg,       metaSize);
        memcpy(data+metaSize, castedMsg->data.data(), castedMsg->dataSize);
      }
      else
        debug_print("msg->dataSize is not matched to real data size");

      castedMsg->data.clear();
    }

  }
}

#endif

