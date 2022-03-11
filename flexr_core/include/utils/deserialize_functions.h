#pragma once

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/types.h"
#include "flexr_core/include/types/frame.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/iostreams/stream.hpp>


namespace flexr {
  namespace utils {

    // deserialize function of kernel ports for FleXR Frame type
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


    // deserialize function of kernel ports for pointer data
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


    // serialize function of kernel ports with boost::serialization
    template <typename T>
    bool deserializeDefault(uint8_t* &data, uint32_t &size, void** msg)
    {
      bool received = true;
      T* castedMsg = static_cast<T*>(*msg);

      boost::iostreams::basic_array_source<char> device((const char*)data, size);
      boost::iostreams::stream<boost::iostreams::basic_array_source<char> > dser(device);
      boost::archive::binary_iarchive ia(dser);
      ia >> *castedMsg;

      delete data;
      data = nullptr;
      return true;
    }

  }
}

