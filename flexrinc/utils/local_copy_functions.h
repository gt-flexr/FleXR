#ifndef __FLEXR_UTILS_LOCALCOPY_FUNCS__
#define __FLEXR_UTILS_LOCALCOPY_FUNCS__

#include "defs.h"
#include "types/cv/types.h"
#include "types/frame.h"
#include "types/types.h"
#include "components/flexr_port.h"

namespace flexr {
  namespace utils {

    template <typename T>
    void sendLocalBasicCopy(components::FleXRPort *port, void *msg)
    {
      T* castedMessage = static_cast<T*>(msg);
      T* copiedMessage = port->getOutputPlaceholder<T>();
      strcpy(copiedMessage->tag, castedMessage->tag);
      copiedMessage->seq  = castedMessage->seq;
      copiedMessage->ts   = castedMessage->ts;
      copiedMessage->data = castedMessage->data;
      port->sendOutput(copiedMessage);
    }


    static void sendLocalFrameCopy(components::FleXRPort *port, void *frame)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(frame);
      types::Message<types::Frame> *copiedFrame = port->getOutputPlaceholder<types::Message<types::Frame>>();

      strcpy(copiedFrame->tag, castedFrame->tag);
      copiedFrame->seq  = castedFrame->seq;
      copiedFrame->ts   = castedFrame->ts;
      copiedFrame->data = castedFrame->data.clone();
      port->sendOutput(copiedFrame);
    }

  }
}

#endif

