#ifndef __MXRE_SENDING_FUNCS__
#define __MXRE_SENDING_FUNCS__

#include<zmq.hpp>
#include"defs.h"
#include"types/cv/types.h"

namespace mxre {
  namespace utils {

    /* sendPrimitive */
    template<typename T>
    void sendPrimitive(T *data, zmq::socket_t *sock) {
      zmq::message_t sendingMsg(data, sizeof(T)), ackMsg;
      sock->send(sendingMsg, zmq::send_flags::none);
      auto res = sock->recv(ackMsg);
    }


    /* sendPrimitiveVector */
    template<typename T>
    void sendPrimitiveVector(T *data, zmq::socket_t *sock) {
      int numOfElem = data->size();
      zmq::message_t sendingMsg(data->begin(), data->end()), ackMsg;

      sock->send(zmq::message_t(&numOfElem, sizeof(int)), zmq::send_flags::sndmore);
      sock->send(sendingMsg, zmq::send_flags::none);
      auto res = sock->recv(ackMsg);
    }


    /* sendDetectedMarkers */
    void sendDetectedMarkers(std::vector<mxre::cv_types::DetectedMarker> *data, zmq::socket_t *sock) {
      zmq::message_t ackMsg;

      // send num of detected markers
      int numOfDetectedMarkers = data->size();
      sock->send(zmq::message_t(&numOfDetectedMarkers, sizeof(int)), zmq::send_flags::none);
      sock->recv(ackMsg);

      for(int i = 0; i < data->size(); i++) {
        zmq::message_t indexMsg(&(*data)[i].index, sizeof(int));

        std::vector<mxre::cv_types::Point3fForCommunication> defaultLocationIn3D;
        std::vector<mxre::cv_types::Point2fForCommunication> locationIn2D;

        for(int j = 0; j < 4; j++) {
          mxre::cv_types::Point3fForCommunication new3DPoint;
          new3DPoint.x = (*data)[i].defaultLocationIn3D[j].x;
          new3DPoint.y = (*data)[i].defaultLocationIn3D[j].y;
          new3DPoint.z = (*data)[i].defaultLocationIn3D[j].z;
          defaultLocationIn3D.push_back(new3DPoint);

          mxre::cv_types::Point2fForCommunication new2DPoint;
          new2DPoint.x = (*data)[i].locationIn2D[j].x;
          new2DPoint.y = (*data)[i].locationIn2D[j].y;
          locationIn2D.push_back(new2DPoint);
        }

        zmq::message_t location3DMsg(defaultLocationIn3D.begin(), defaultLocationIn3D.end());
        zmq::message_t location2DMsg(locationIn2D.begin(),locationIn2D.end());

        sock->send(indexMsg, zmq::send_flags::sndmore);
        sock->send(location3DMsg, zmq::send_flags::sndmore);
        sock->send(location2DMsg, zmq::send_flags::none);

        auto res = sock->recv(ackMsg);
      }
    }

  }
}

#endif

