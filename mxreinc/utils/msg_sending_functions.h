#ifndef __MXRE_UTILS_MSGSENDINGFUNCS__
#define __MXRE_UTILS_MSGSENDINGFUNCS__

#include <zmq.hpp>
#include "defs.h"
#include "types/cv/types.h"
#include "types/frame.h"
#include "types/types.h"
#include "components/mxre_port.h"

namespace mxre {
  namespace utils {

    template <typename T>
    void sendLocalBasicCopy(components::MXREPort *port, void *msg)
    {
      T* castedMessage = static_cast<T*>(msg);
      T* copiedMessage = port->getOutputPlaceholder<T>();
      strcpy(copiedMessage->tag, castedMessage->tag);
      copiedMessage->seq  = castedMessage->seq;
      copiedMessage->ts   = castedMessage->ts;
      copiedMessage->data = castedMessage->data;
      port->sendOutput(copiedMessage);
    }

    static void sendLocalFrameCopy(components::MXREPort *port, void *frame)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(frame);
      types::Message<types::Frame> *copiedFrame = port->getOutputPlaceholder<types::Message<types::Frame>>();

      strcpy(copiedFrame->tag, castedFrame->tag);
      copiedFrame->seq  = castedFrame->seq;
      copiedFrame->ts   = castedFrame->ts;
      copiedFrame->data = castedFrame->data.clone();
      port->sendOutput(copiedFrame);
    }

    template <typename T>
    void sendRemotePrimitive(components::MXREPort *port, void *msg)
    {
      zmq::message_t sendMsg(msg, sizeof(T));
      port->remotePort.socket.send(sendMsg, zmq::send_flags::none);
      T* castedMessage = static_cast<T*>(msg);
    }

    template <typename T>
    void sendRemotePrimitiveVec(components::MXREPort *port, void *msg)
    {
      T* castedMsg = static_cast<T*>(msg);
      int vecSize = castedMsg->data.size();
      zmq::message_t sizeMsg(&vecSize, sizeof(int));
      zmq::message_t sendMsg(castedMsg->data);

      port->remotePort.socket.send(sendMsg, zmq::send_flags::none);

      port->remotePort.socket.send(zmq::message_t(castedMsg->tag, MXRE_MSG_TAG_SIZE), zmq::send_flags::sndmore);
      port->remotePort.socket.send(zmq::message_t(&castedMsg->seq, sizeof(castedMsg->seq)), zmq::send_flags::sndmore);
      port->remotePort.socket.send(zmq::message_t(&castedMsg->ts, sizeof(castedMsg->ts)), zmq::send_flags::sndmore);
      port->remotePort.socket.send(zmq::message_t(&vecSize, sizeof(int)), zmq::send_flags::sndmore);
      port->remotePort.socket.send(sendMsg, zmq::send_flags::none);
    }

    /* sendDetectedMarkers */
    using SendMarkerMsgType = types::Message<std::vector<cv_types::DetectedMarker>>;
    static void sendRemoteMarkers(components::MXREPort *port, void *data)
    {
      SendMarkerMsgType *castedMsg = static_cast<SendMarkerMsgType*>(data);

      int vecSize = castedMsg->data.size();
      port->remotePort.socket.send(zmq::message_t(castedMsg->tag, MXRE_MSG_TAG_SIZE), zmq::send_flags::sndmore);
      port->remotePort.socket.send(zmq::message_t(&castedMsg->seq, sizeof(castedMsg->seq)), zmq::send_flags::sndmore);
      port->remotePort.socket.send(zmq::message_t(&castedMsg->ts, sizeof(castedMsg->ts)), zmq::send_flags::sndmore);
      port->remotePort.socket.send(zmq::message_t(&vecSize, sizeof(int)), zmq::send_flags::sndmore);

      for(int i = 0; i < vecSize; i++) {
        zmq::message_t indexMsg(&(castedMsg->data)[i].index, sizeof(int));

        std::vector<mxre::cv_types::Point3fForCommunication> defaultLocationIn3D;
        std::vector<mxre::cv_types::Point2fForCommunication> locationIn2D;

        for(int j = 0; j < 4; j++) {
          mxre::cv_types::Point3fForCommunication new3DPoint;
          new3DPoint.x = (castedMsg->data)[i].defaultLocationIn3D[j].x;
          new3DPoint.y = (castedMsg->data)[i].defaultLocationIn3D[j].y;
          new3DPoint.z = (castedMsg->data)[i].defaultLocationIn3D[j].z;
          defaultLocationIn3D.push_back(new3DPoint);

          mxre::cv_types::Point2fForCommunication new2DPoint;
          new2DPoint.x = (castedMsg->data)[i].locationIn2D[j].x;
          new2DPoint.y = (castedMsg->data)[i].locationIn2D[j].y;
          locationIn2D.push_back(new2DPoint);
        }

        zmq::message_t location3DMsg(defaultLocationIn3D.begin(), defaultLocationIn3D.end());
        zmq::message_t location2DMsg(locationIn2D.begin(),locationIn2D.end());

        port->remotePort.socket.send(indexMsg, zmq::send_flags::sndmore);
        port->remotePort.socket.send(location3DMsg, zmq::send_flags::sndmore);
        port->remotePort.socket.send(location2DMsg, zmq::send_flags::none);
      }
    }

  }
}

#endif

