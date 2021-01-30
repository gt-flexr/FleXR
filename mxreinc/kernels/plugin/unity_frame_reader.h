#ifndef __MXRE_UNITY_FRAME_READER__
#define __MXRE_UNITY_FRAME_READER__

#include <bits/stdc++.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <opencv/cv.hpp>
#include "kernels/kernel.h"
#include "defs.h"
#include "types/frame.h"

namespace mxre {
  namespace kernels {

    class UnityFrameReader : public mxre::kernels::MXREKernel{
    private:
      zmq::context_t ctx;
      zmq::socket_t sock;
      int sockType;

    public:
      UnityFrameReader(int id = 0, int zmqSockType = ZMQ_REP);
      ~UnityFrameReader();
      virtual raft::kstatus run();
    };


    /* UnityFrameReader */
    UnityFrameReader::UnityFrameReader(int id, int zmqSockType): MXREKernel(), sockType(zmqSockType) {
      sock = zmq::socket_t(ctx, zmqSockType);
      std::string readingAddr = "ipc:///tmp/mxre_unity_frame_read:" + std::to_string(id);
      switch(zmqSockType) {
      case ZMQ_REP:
        sock.bind(readingAddr);
        break;
      case ZMQ_SUB:
        sock.connect(readingAddr);
        break;
      }
      addOutputPort<mxre::types::Frame>("out_frame");
    }


    /* ~UnityFrameReader */
    UnityFrameReader::~UnityFrameReader() {
      sock.close();
      ctx.shutdown();
      ctx.close();
    }


    /* Run */
    raft::kstatus UnityFrameReader::run() {
#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      auto &outFrame(output["out_frame"].allocate<mxre::types::Frame>());

      zmq::recv_multipart_n(sock, outFrame, sizeof(mxre::types::Frame));
      outFrame.data = new unsigned char[outFrame.dataSize];
      zmq::recv_multipart_n(sock, outFrame.data, outFrame.dataSize);
      if(sockType == ZMQ_REQ) {
        zmq::message_t ackMsg("ack", 3);
        sock.send(ackMsg, zmq::send_flags::none);
      }

      output["out_frame"].send();
      sendFrameCopy("out_frame", &outFrame);

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }
  }
}

#endif

