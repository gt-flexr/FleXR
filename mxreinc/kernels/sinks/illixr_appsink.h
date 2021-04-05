#ifndef __MXRE_ILLIXR_APP_SINK__
#define __MXRE_ILLIXR_APP_SINK__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/clock_types.h"
#include "types/cv/types.h"
#include "types/kimera/types.h"
#include "utils/cv_utils.h"
#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {

    template<typename IN_T>
    class IllixrAppSink : public MXREKernel
    {
    protected:
      void *ctx;
      void *sock;
      int dtype;
      char ack[3];

    public:
      IllixrAppSink() {
        addInputPort<IN_T>("in_data");
      }


      ~IllixrAppSink() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      void setup(std::string id, int dtype=MXRE_DTYPE_PRIMITIVE) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);
        std::string connectAddr = "ipc:///tmp/" + id;
        zmq_connect(sock, connectAddr.c_str());

        debug_print("connectAddr: %s bound\n", connectAddr.c_str());
      }


      void sendPrimitive(IN_T *data) {
        zmq_send(sock, data, sizeof(IN_T), 0);
      }


      void sendFrame(IN_T *data) {
        mxre::types::Frame *frame = (mxre::types::Frame*)data;
        //cv::Mat *mat = (cv::Mat*)data;
        //mxre::types::Frame frame;
        //frame.setFrameAttribFromCVMat(*mat);
        debug_print("%d %d %d %d", frame->cols, frame->rows, frame->dataSize, frame->totalElem);

        zmq_send(sock, frame, sizeof(mxre::types::Frame), ZMQ_SNDMORE);
        zmq_send(sock, frame->data, frame->dataSize, 0);
        frame->release();
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif
        if(sock == NULL || ctx == NULL) {
          debug_print("AppSource is not set");
          return raft::stop;
        }

        auto &inData( input["in_data"].template peek<IN_T>() );

        switch(dtype) {
          case MXRE_DTYPE_PRIMITIVE:
            sendPrimitive(&inData);
            break;
          case MXRE_DTYPE_FRAME:
            sendFrame(&inData);
            break;
        }
        zmq_recv(sock, ack, sizeof(ack), 0);

#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

        input["in_data"].recycle();
        return raft::proceed;
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

