#ifndef __MXRE_APP_SOURCE__
#define __MXRE_APP_SOURCE__

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

    template<typename OUT_T>
    class AppSource : public raft::kernel
    {
    protected:
      void *ctx;
      void *sock;

    public:
      AppSource(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REP);

        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());

        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());

        output.addPort<OUT_T>("out_data");
      }


      ~AppSource() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif
        auto &outData( output["out_data"].template allocate<OUT_T>() );
        zmq_recv(sock, &outData, sizeof(OUT_T), 0);
        zmq_send(sock, "ack", 3, 0);
        debug_print("recv: %d \n", outData);

#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

        output["out_data"].send();
        return raft::proceed;
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

