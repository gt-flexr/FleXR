#ifndef __MXRE_ILLIXR_APP_ZMQ_SINK__
#define __MXRE_ILLIXR_APP_ZMQ_SINK__

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
    class IllixrAppZMQSink : public MXREKernel
    {
    protected:
      void *ctx;
      void *sock;
      int dtype;
      char ack[4];

    public:
      IllixrAppZMQSink() {
        addInputPort<IN_T>("in_data");
      }


      ~IllixrAppZMQSink() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      void setup(char const* dest_machine_ip) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_PAIR);
        std::string dest_machine_ip_str(dest_machine_ip);
        std::string connectAddr = std::string("tcp://")+dest_machine_ip_str+std::string(":19435");
        zmq_connect(sock, connectAddr.c_str());

        debug_print("connectAddr: %s bound\n", connectAddr.c_str());
      }


      void sendKimeraOutput(IN_T* output_data_) {
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSink is not set." << std::endl;
          return;
        }

        mxre::kimera_type::kimera_output *output_data = (mxre::kimera_type::kimera_output*) output_data_;
        zmq_send(sock, output_data, sizeof(mxre::kimera_type::kimera_output), 0);
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif
	      // debug_print("MXRE SENDING DATA TO ILLIXR (3)");

        if(sock == NULL || ctx == NULL) {
          debug_print("AppSource is not set");
          return raft::stop;
        }

        auto &inData( input["in_data"].template peek<IN_T>() );
        sendKimeraOutput(&inData);
        // zmq_recv(sock, ack, sizeof(ack), 0);

#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        // profile_print("Exe Time ILLIXR Appsink: %lfms", getExeTime(end, start));
#endif

        input["in_data"].recycle();
        return raft::proceed;
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

