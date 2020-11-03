#ifndef __MXRE_CVMAT_APP_SINK__
#define __MXRE_CVMAT_APP_SINK__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/clock_types.h"
#include "types/cv/types.h"
#include "utils/cv_utils.h"

namespace mxre
{
  namespace kernels
  {

    class CVMatAppSink : public raft::kernel
    {
    protected:
      void *ctx;
      void *sock;
      char ack[3];

    public:
      CVMatAppSink(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REQ);

        std::string connectAddr = "ipc:///tmp/" + id;
        zmq_connect(sock, connectAddr.c_str());

        debug_print("connectAddr: %s bound\n", connectAddr.c_str());

        input.addPort<mxre::cv_types::Mat>("in_data");
      }


      ~CVMatAppSink() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif

        auto &inData( input["in_data"].template peek<mxre::cv_types::Mat>() );
        uint matInfo[MX_MAT_ATTR_NUM];
        matInfo[MX_MAT_SIZE_IDX] = inData.total * inData.elemSize;
        matInfo[MX_MAT_ROWS_IDX] = inData.rows;
        matInfo[MX_MAT_COLS_IDX] = inData.cols;
        matInfo[MX_MAT_TYPE_IDX] = inData.type;

        zmq_send(sock, matInfo, sizeof(matInfo), ZMQ_SNDMORE);
        zmq_send(sock, inData.data, matInfo[MX_MAT_SIZE_IDX], 0);
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

