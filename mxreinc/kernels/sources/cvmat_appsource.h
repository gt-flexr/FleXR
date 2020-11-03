#ifndef __MXRE_CVMAT_APP_SOURCE__
#define __MXRE_CVMAT_APP_SOURCE__

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

    class CVMatAppSource : public raft::kernel
    {
    protected:
      void *ctx;
      void *sock;

    public:
      CVMatAppSource(std::string id) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_REP);

        std::string bindingAddr = "ipc:///tmp/" + id;
        zmq_bind(sock, bindingAddr.c_str());

        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());

        output.addPort<mxre::cv_types::Mat>("out_data");
      }


      ~CVMatAppSource() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif
        auto &outData( output["out_data"].template allocate<mxre::cv_types::Mat>() );
        uint matInfo[MX_MAT_ATTR_NUM];
        void *recvData;

        zmq_recv(sock, matInfo, sizeof(matInfo), 0);
        recvData = new uchar[matInfo[MX_MAT_SIZE_IDX]];

        zmq_recv(sock, recvData, matInfo[MX_MAT_SIZE_IDX], 0);
        zmq_send(sock, "ack", 3, 0);

        outData = mxre::cv_types::Mat(matInfo[MX_MAT_ROWS_IDX], matInfo[MX_MAT_COLS_IDX], matInfo[MX_MAT_TYPE_IDX],
                                    recvData);

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

