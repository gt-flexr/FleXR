#ifndef __MXRE_COMP_YOLO_PREDICTION_RECEIVER__
#define __MXRE_COMP_YOLO_PREDICTION_RECEIVER__

#include <bits/stdc++.h>
#include <zmq.h>
#include "defs.h"
#include "network/message_receiver.h"
#include "perceptions/complex_yolo/types.h"

namespace mxre {
  namespace perception {
    namespace complex_yolo {

      template<typename RECV_T>
      class PredictionReceiver : public pipeline::network::MessageReceiver {
        public:
        PredictionReceiver(int port=5555) : pipeline::network::MessageReceiver(port){
          output.addPort<std::vector<RECV_T>>("out_predictions");
        }

        raft::kstatus run() {

          int num;
          zmq_recv(sock, &num, sizeof(int), 0);
          zmq_send(sock, "ack", strlen("ack"), 0);

#ifdef __PROFILE__
          TimeVal start = getNow();
#endif

          auto &predictions( output["out_predictions"].template allocate<std::vector<RECV_T>>() );
          for(int i = 0; i < num; i++) {
            RECV_T pred;
            zmq_recv(sock, &pred, sizeof(Prediction), 0);
            zmq_send(sock, "ack", strlen("ack"), 0);
            predictions.push_back(pred);
          }

          output["out_predictions"].send();
          return raft::proceed;
        }
      };

    }
  } // namespace perception
} // namespace mxre

#endif

