#ifndef __MXRE_RECEIVING_FUNCS__
#define __MXRE_RECEIVING_FUNCS__

#include<zmq.hpp>
#include<zmq_addon.hpp>
#include"defs.h"
#include"types/cv/types.h"

namespace mxre {
  namespace utils {

    /* recvPrimitive */
    template<typename T>
    void recvPrimitive(T *data, zmq::socket_t *sock) {
      zmq::message_t ackMsg("ack", 3);
      sock->recv( zmq::buffer(data, sizeof(T)) );
      sock->send(ackMsg, zmq::send_flags::none);
    }


    /* recvPrimitiveVector */
    template<typename T>
    void recvPrimitiveVector(T *data, zmq::socket_t *sock) {
      int numOfElem;
      zmq::message_t ackMsg("ack", 3);

      sock->recv(zmq::buffer(&numOfElem, sizeof(int)), zmq::recv_flags::none);
      data->resize(numOfElem);
      sock->recv(zmq::buffer(*data), zmq::recv_flags::none);

      sock->send(ackMsg, zmq::send_flags::none);
    }


    /* recvDetectedMarkers */
    void recvDetectedMarkers(std::vector<mxre::cv_types::DetectedMarker> *data, zmq::socket_t *sock) {
      zmq::message_t ackMsg("ack", 3);
      int numOfDetectedMarkers;

      sock->recv(zmq::buffer(&numOfDetectedMarkers, sizeof(int)), zmq::recv_flags::none);
      sock->send(ackMsg, zmq::send_flags::none);

      for(int i = 0; i < numOfDetectedMarkers; i++) {
        mxre::cv_types::DetectedMarker detectedMarker;
        //detectedMarker.defaultLocationIn3D.resize(4);
        //detectedMarker.locationIn2D.resize(4);
        std::vector<mxre::cv_types::Point3fForCommunication> defaultLocationIn3D;
        std::vector<mxre::cv_types::Point2fForCommunication> locationIn2D;
        defaultLocationIn3D.resize(4);
        locationIn2D.resize(4);

        //zmq::message_t defaultLocationIn3DMsg(defaultLocationIn3D.begin(), defaultLocationIn3D.end());
        //zmq::message_t locationIn2DMsg(locationIn2D.begin(), locationIn2D.end());

        sock->recv(zmq::buffer(&detectedMarker.index, sizeof(int)), zmq::recv_flags::none);
        sock->recv(zmq::buffer(defaultLocationIn3D), zmq::recv_flags::none);
        sock->recv(zmq::buffer(locationIn2D), zmq::recv_flags::none);
        sock->send(ackMsg, zmq::send_flags::none);
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

        data->push_back(detectedMarker);
      }
    }

  }
}

#endif

