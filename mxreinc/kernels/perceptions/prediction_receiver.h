#ifndef __MXRE_COMP_YOLO_PREDICTION_RECEIVER__
#define __MXRE_COMP_YOLO_PREDICTION_RECEIVER__

#include <bits/stdc++.h>
#include <zmq.h>
#include "defs.h"
#include "kernels/offloading/message_receiver.h"
#include "types/complex_yolo_types.h"
#include "types/clock_types.h"

namespace mxre {
  namespace kernels {

    template<typename RECV_T>
    class PredictionReceiver : public MXREKernel{
      public:
      PredictionReceiver(int port=5555) : MXREKernel(){
        output.addPort<std::vector<RECV_T>>("out_predictions");
      }

      raft::kstatus run() {
        int num;
        /*
        zmq_recv(sock, &num, sizeof(int), 0);
        zmq_send(sock, "ack", strlen("ack"), 0);

#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif

        auto &predictions( output["out_predictions"].template allocate<std::vector<RECV_T>>() );
        for(int i = 0; i < num; i++) {
          RECV_T pred;
          zmq_recv(sock, &pred, sizeof(mxre::types::ComplexYoloPrediction), 0);
          zmq_send(sock, "ack", strlen("ack"), 0);
          predictions.push_back(pred);
        }

        output["out_predictions"].send();*/
        return raft::proceed;
      }
    };

  } // namespace kernels
} // namespace mxre

#endif

