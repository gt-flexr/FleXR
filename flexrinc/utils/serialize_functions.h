#ifndef __FLEXR_UTILS_SERIALIZE_FUNCS__
#define __FLEXR_UTILS_SERIALIZE_FUNCS__

#include <zmq.hpp>
#include "defs.h"
#include "types/frame.h"
#include "types/types.h"
#include "components/flexr_port.h"

namespace flexr {
  namespace utils {

    static void serializeRawFrame(void* msg, uint8_t* &data, uint32_t &size)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(msg);
      uint32_t msgMetaSize = sizeof(*castedFrame);
      uint32_t frameMetaSize = sizeof(types::Frame);

      size = msgMetaSize + frameMetaSize + castedFrame->dataSize;

      if(castedFrame->dataSize != castedFrame->data.dataSize &&
         castedFrame->dataSize != castedFrame->data.useAsCVMat().total()*castedFrame->data.useAsCVMat().elemSize())
      {
        debug_print("Message<Frame> dataSize(%d) mismatches to Frame->dataSize (%ld) and real dataSize(%ld)",
                    castedFrame->dataSize, castedFrame->data.dataSize,
                    castedFrame->data.useAsCVMat().total()*castedFrame->data.useAsCVMat().elemSize());
      }

      data = new uint8_t[size];
      uint8_t *temp = data;
      memcpy(temp, castedFrame,            msgMetaSize);   temp += msgMetaSize;
      memcpy(temp, &castedFrame->data,     frameMetaSize); temp += frameMetaSize;
      memcpy(temp, castedFrame->data.data, castedFrame->dataSize);

      castedFrame->data.release();
    }


    static void serializeEncodedFrame(void* msg, uint8_t* &data, uint32_t &size)
    {
      types::Message<uint8_t*> *castedFrame = static_cast<types::Message<uint8_t*>*>(msg);
      uint32_t metaSize = sizeof(*castedFrame);

      size = metaSize + castedFrame->dataSize;
      data = new uint8_t[size];

      memcpy(data,          castedFrame,       metaSize);
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

