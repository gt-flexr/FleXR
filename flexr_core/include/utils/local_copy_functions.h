#ifndef __FLEXR_CORE_UTILS_LOCALCOPY_FUNCS__
#define __FLEXR_CORE_UTILS_LOCALCOPY_FUNCS__

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/frame.h"
#include "flexr_core/include/types/types.h"
#include "flexr_core/include/components/flexr_port.h"

namespace flexr {
  namespace utils {

    template <typename T>
    void sendLocalBasicCopy(components::FleXRPort *port, void *msg)
    {
      T* castedMessage = static_cast<T*>(msg);
      T* copiedMessage = port->getOutputPlaceholder<T>();

      copiedMessage->setHeader(castedMessage->tag, castedMessage->seq, castedMessage->ts, castedMessage->dataSize);
      copiedMessage->data = castedMessage->data;

      port->sendOutput(copiedMessage);
    }


    static void sendLocalPointerMsgCopy(components::FleXRPort *port, void *msg)
    {
      types::Message<uint8_t*> *castedMessage = static_cast<types::Message<uint8_t*>*>(msg);
      types::Message<uint8_t*> *copiedMessage = port->getOutputPlaceholder<types::Message<uint8_t*>>();

      copiedMessage->setHeader(castedMessage->tag, castedMessage->seq, castedMessage->ts, castedMessage->dataSize);
      copiedMessage->data = new uint8_t[copiedMessage->dataSize];
      memcpy(copiedMessage->data, castedMessage->data, castedMessage->dataSize);
      port->sendOutput(copiedMessage);
    }


    static void sendLocalFrameCopy(components::FleXRPort *port, void *frame)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(frame);
      types::Message<types::Frame> *copiedFrame = port->getOutputPlaceholder<types::Message<types::Frame>>();

      copiedFrame->setHeader(castedFrame->tag, castedFrame->seq, castedFrame->ts, castedFrame->dataSize);
      copiedFrame->data = castedFrame->data.clone();
      port->sendOutput(copiedFrame);
    }

  }
}

#endif

