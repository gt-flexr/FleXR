#ifndef __MXRE_APP_SINK__
#define __MXRE_APP_SINK__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/clock_types.h"
#include "utils/cv_utils.h"

namespace mxre
{
  namespace kernels
  {

    template<typename IN_T>
    class AppSink : public raft::kernel
    {
    protected:
      void *ctx;
      void *sock;

    public:
      AppSink(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);

        std::string connectAddr = "ipc:///tmp/" + id;
        zmq_connect(sock, connectAddr.c_str());

        debug_print("connectAddr: %s bound\n", connectAddr.c_str());

        input.addPort<IN_T>("in_data");
      }


      ~AppSink() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      virtual raft::kstatus run() {
        char ack[4];
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif
        auto &inData( input["in_data"].template peek<IN_T>() );
        debug_print("sink: %d", inData);
        zmq_send(sock, (void*)&inData, sizeof(IN_T), 0);
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

