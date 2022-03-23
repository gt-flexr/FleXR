#ifndef __FLEXR_CORE_TYPES__
#define __FLEXR_CORE_TYPES__

#include <bits/stdc++.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/frame.h"

// TODO: integrate all the types into this file
namespace flexr {
  namespace types {
    template <class T>
    class Message
    {
      public:
      char     tag[FLEXR_MSG_TAG_SIZE];
      uint32_t seq;
      uint32_t ts;
      uint32_t dataSize;
      T        data;

      void printHeader()
      {
        printf("Message Header: %s \n", tag);
        printf("\tseq: %d, ts: %ld, dataSize %d\n", seq, ts, dataSize);
      }

      void setHeader(std::string tag, uint32_t seq, uint32_t ts, uint32_t dataSize)
      {
        std::strcpy(this->tag, tag.c_str());
        this->seq      = seq;
        this->ts       = ts;
        this->dataSize = dataSize;
      }

      private:
      friend class boost::serialization::access;
      template <class Archieve>
      void serialize(Archieve &ar, const unsigned int version)
      {
        ar& tag;
        ar& seq;
        ar& ts;
        ar& dataSize;
        ar& data;
      }
    };


    class ObjectPose
    {
      public:
      float rx, ry, rz;
      float tx, ty, tz;
    };


    class ImuPoint
    {
      private:
        friend class boost::serialization::access;
        template<class Archieve>
          void serialize(Archieve&ar, const unsigned int version)
          {
            ar& a.x;
            ar& a.y;
            ar& a.z;
            ar& g.x;
            ar& g.y;
            ar& g.z;
            ar& t;
          }


      public:
        ImuPoint(){}
        ImuPoint(const float &acc_x, const float &acc_y, const float &acc_z,
            const float &ang_vel_x, const float &ang_vel_y, const float &ang_vel_z,
            const double &timestamp): a(acc_x,acc_y,acc_z), g(ang_vel_x,ang_vel_y,ang_vel_z), t(timestamp){}
        ImuPoint(const cv::Point3f Acc, const cv::Point3f Gyro, const double &timestamp):
          a(Acc.x,Acc.y,Acc.z), g(Gyro.x,Gyro.y,Gyro.z), t(timestamp){}
      public:
        cv::Point3f a;
        cv::Point3f g;
        double t;
    };


    template <typename T>
    void freePrimitiveMsg(void *msg)
    {
      T* castedMessage = static_cast<T*>(msg);
      delete castedMessage;
    }


    static void freeFrameMsg(void *msg)
    {
      Message<Frame> *castedFrame = static_cast<Message<Frame>*>(msg);
      castedFrame->data.release();
      delete castedFrame;
    }


    static void allocFrameWithBuffer(void **msg, int size)
    {
      Message<Frame> *castedFrame = new Message<Frame>();
      castedFrame->data.data     = new unsigned char[size];
      castedFrame->data.dataSize = size;
      *msg = castedFrame;
    }
  }
}

#endif

