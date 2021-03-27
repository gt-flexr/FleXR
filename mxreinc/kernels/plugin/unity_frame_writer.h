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
      zmq::socket_t publisher;

    public:
      UnityFrameWriter(int id = 0);
      ~UnityFrameWriter();
      virtual raft::kstatus run();
    };


    /* UnityFrameWriter */
    UnityFrameWriter::UnityFrameWriter(int id): MXREKernel() {
      publisher = zmq::socket_t(ctx, zmq::socket_type::pub);
      std::string bindingAddr = "tcp://*:" + std::to_string(id);
      publisher.bind(bindingAddr);

      addInputPort<mxre::types::Frame>("in_frame");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("unity_frame_writer", "logs/" + std::to_string(pid) + "/unity_frame_writer.log");
#endif

    }


    /* ~UnityFrameWriter */
    UnityFrameWriter::~UnityFrameWriter() {
      publisher.close();
      ctx.shutdown();
      ctx.close();
    }


    /* Run */
    raft::kstatus UnityFrameWriter::run() {
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      auto &inFrame(input["in_frame"].peek<mxre::types::Frame>());

      zmq::message_t frameData(inFrame.data, inFrame.dataSize);
      publisher.send(frameData, zmq::send_flags::none);
      debug_print("publish a frame to unity");

      inFrame.release();
      recyclePort("in_frame");

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif

      return raft::proceed;
    }
  }
}

#endif

