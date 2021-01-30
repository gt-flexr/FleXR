#ifndef __MXRE_UNITY_FRAME_WRITER__
#define __MXRE_UNITY_FRAME_WRITER__

#include <bits/stdc++.h>
#include <zmq.hpp>
#include <opencv/cv.hpp>
#include "kernels/kernel.h"
#include "defs.h"
#include "types/frame.h"

namespace mxre {
  namespace kernels {

    class UnityFrameWriter : public mxre::kernels::MXREKernel{
    private:
      zmq::context_t ctx;
      zmq::socket_t sock;
      int sockType;

    public:
      UnityFrameWriter(int id = 0, int zmqSockType = ZMQ_REQ);
      ~UnityFrameWriter();
      virtual raft::kstatus run();
    };


    /* UnityFrameWriter */
    UnityFrameWriter::UnityFrameWriter(int id, int zmqSockType): MXREKernel(), sockType(zmqSockType) {
      sock = zmq::socket_t(ctx, zmqSockType);
      std::string writingAddr = "ipc:///tmp/mxre_unity_frame_write:" + std::to_string(id);
      switch(zmqSockType) {
      case ZMQ_REQ:
        sock.connect(writingAddr);
        break;
      case ZMQ_PUB:
        sock.bind(writingAddr);
        break;
      }
      addInputPort<mxre::types::Frame>("in_frame");
    }


    /* ~UnityFrameWriter */
    UnityFrameWriter::~UnityFrameWriter() {
      sock.close();
      ctx.shutdown();
      ctx.close();
    }


    /* Run */
    raft::kstatus UnityFrameWriter::run() {
#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      auto &inFrame(input["in_frame"].peek<mxre::types::Frame>());
      zmq::message_t frameInfo(&inFrame, sizeof(inFrame));
      zmq::message_t frameData(inFrame.data, inFrame.dataSize);

      sock.send(frameInfo, zmq::send_flags::sndmore);
      sock.send(frameData, zmq::send_flags::none);

      if(sockType == ZMQ_REQ) {
        zmq::message_t ackMsg;
        sock.recv(ackMsg);
      }
      inFrame.release();
      recyclePort("in_frame");

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }
  }
}

#endif

