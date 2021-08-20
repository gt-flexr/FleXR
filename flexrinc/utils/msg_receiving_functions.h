#ifndef __FLEXR_RECEIVING_FUNCS__
#define __FLEXR_RECEIVING_FUNCS__

#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "defs.h"
#include "types/cv/types.h"
#include "types/types.h"
#include "types/frame.h"
#include "components/flexr_port.h"

namespace flexr {
  namespace utils {

    template <typename T>
    void recvRemotePrimitive(components::FleXRPort *port, void *msg)
    {
      port->remotePort.socket.recv(zmq::buffer(msg, sizeof(T)));
    }


    static void recvRemoteFrame(components::FleXRPort *port, void *msg)
    {
      types::Message<types::Frame> *castedFrame = static_cast<types::Message<types::Frame>*>(msg);
      port->remotePort.socket.recv(zmq::buffer(castedFrame->data.data, castedFrame->data.dataSize));
    }


    template <typename T>
    void recvRemotePrimitiveVec(components::FleXRPort *port, void *msg)
    {
      T* castedMsg = static_cast<T*>(msg);
      int vecSize = 0;
      port->remotePort.socket.recv(zmq::buffer(castedMsg->tag, FLEXR_MSG_TAG_SIZE), zmq::recv_flags::none);
      port->remotePort.socket.recv(zmq::buffer(&castedMsg->seq, sizeof(castedMsg->seq)), zmq::recv_flags::none);
      port->remotePort.socket.recv(zmq::buffer(&castedMsg->ts, sizeof(castedMsg->ts)), zmq::recv_flags::none);
      port->remotePort.socket.recv(zmq::buffer(&vecSize, sizeof(int)), zmq::recv_flags::none);
      castedMsg->data.resize(vecSize);
      port->remotePort.socket.recv(zmq::buffer(castedMsg->data), zmq::recv_flags::none);
    }

    template <typename T>
    void recvNonBlockRemotePrimitive(components::FleXRPort *port, void *msg)
    {
      port->remotePort.socket.recv(zmq::buffer(msg, sizeof(T)), zmq::recv_flags::dontwait);
    }

    template <typename T>
    void recvNonBlockRemotePrimitiveVec(components::FleXRPort *port, void *msg)
    {
      T* castedMsg = static_cast<T*>(msg);
      int vecSize = 0;
      port->remotePort.socket.recv(zmq::buffer(castedMsg->tag, FLEXR_MSG_TAG_SIZE), zmq::recv_flags::dontwait);
      port->remotePort.socket.recv(zmq::buffer(&castedMsg->seq, sizeof(castedMsg->seq)), zmq::recv_flags::dontwait);
      port->remotePort.socket.recv(zmq::buffer(&castedMsg->ts, sizeof(castedMsg->ts)), zmq::recv_flags::dontwait);
      port->remotePort.socket.recv(zmq::buffer(&vecSize, sizeof(int)), zmq::recv_flags::dontwait);
      castedMsg->data.resize(vecSize);
      port->remotePort.socket.recv(zmq::buffer(castedMsg->data), zmq::recv_flags::dontwait);
    }

    /* recvDetectedMarkers */
    using RecvMarkerMsgType = types::Message<std::vector<cv_types::DetectedMarker>>;
    static void recvDetectedMarkers(components::FleXRPort *port, void *data)
    {
      RecvMarkerMsgType *castedMsg = static_cast<RecvMarkerMsgType*>(data);

      int vecSize = 0;
      port->remotePort.socket.recv(zmq::buffer(castedMsg->tag, FLEXR_MSG_TAG_SIZE), zmq::recv_flags::none);
      port->remotePort.socket.recv(zmq::buffer(&castedMsg->seq, sizeof(castedMsg->seq)), zmq::recv_flags::none);
      port->remotePort.socket.recv(zmq::buffer(&castedMsg->ts, sizeof(castedMsg->ts)), zmq::recv_flags::none);
      port->remotePort.socket.recv(zmq::buffer(&vecSize, sizeof(int)), zmq::recv_flags::none);

      for(int i = 0; i < vecSize; i++) {
        flexr::cv_types::DetectedMarker detectedMarker;
        std::vector<flexr::cv_types::Point3fForCommunication> defaultLocationIn3D;
        std::vector<flexr::cv_types::Point2fForCommunication> locationIn2D;
        defaultLocationIn3D.resize(4);
        locationIn2D.resize(4);

        port->remotePort.socket.recv(zmq::buffer(&detectedMarker.index, sizeof(int)), zmq::recv_flags::none);
        port->remotePort.socket.recv(zmq::buffer(defaultLocationIn3D), zmq::recv_flags::none);
        port->remotePort.socket.recv(zmq::buffer(locationIn2D), zmq::recv_flags::none);
        for(int i = 0; i < 4; i++) {
          cv::Point3f new3DPoint;
          new3DPoint.x = defaultLocationIn3D[i].x;
          new3DPoint.y = defaultLocationIn3D[i].y;
          new3DPoint.z = defaultLocationIn3D[i].z;
          detectedMarker.defaultLocationIn3D.push_back(new3DPoint);

          cv::Point2f new2DPoint;
          new2DPoint.x = locationIn2D[i].x;
          new2DPoint.y = locationIn2D[i].y;
          detectedMarker.locationIn2D.push_back(new2DPoint);
        }

        castedMsg->data.push_back(detectedMarker);
      }
    }

  }
}

#endif

