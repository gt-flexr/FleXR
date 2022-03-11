#pragma once

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/frame.h"
#include "flexr_core/include/types/types.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/iostreams/stream.hpp>


namespace flexr {
  namespace utils {

    // serialize function of kernel ports for FleXR Frame type
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


    // serialize function of kernel ports for pointer data
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


    // serialize function of kernel ports for defulat types with boost::serialization
    template <typename T>
    bool serializeDefault(void* msg, uint8_t* &data, uint32_t &size, bool freeMsgData)
    {
      T* castedMsg = static_cast<T*>(msg);

      std::string serStr;
      boost::iostreams::back_insert_device<std::string> inserter(serStr);
      boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
      boost::archive::binary_oarchive oa(s);
      oa << *castedMsg;
      s.flush();

      size = serStr.size();
      data = new uint8_t[size];
      memcpy(data, serStr.data(), size);

      return true;
    }


    // serialize function of kernel ports for vector types with boost::serialization
    template <typename T>
    bool serializeVector(void* msg, uint8_t* &data, uint32_t &size, bool freeMsgData)
    {
      serializeDefault<T>(msg, data, size, freeMsgData);

      if(freeMsgData)
      {
        T* castedMsg = static_cast<T*>(msg);
        castedMsg->data.clear();
      }
      return true;
    }

  }
}

