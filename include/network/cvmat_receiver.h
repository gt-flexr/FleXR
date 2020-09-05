#ifndef __MXRE_CVMAT_RECEIVER__
#define __MXRE_CVMAT_RECEIVER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <cv.hpp>
#include "defs.h"
#include "message_receiver.h"

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      /* Class Deifinition */
      class CVMatReceiver : public MessageReceiver
      {
        private:
          bool isVector;
        public:
          CVMatReceiver(int port=5555, bool isVector=false);
          virtual raft::kstatus run();
      };


      /* Constructor() */
      CVMatReceiver::CVMatReceiver(int port, bool isVector) : MessageReceiver(port) {
        this->isVector = isVector;
        if(isVector)
          output.addPort<std::vector<cv::Mat>>("out_data");
        else
          output.addPort<cv::Mat>("out_data");
      }


      /* Run() */
      raft::kstatus CVMatReceiver::run() {
        debug_print("[CVMatReceiver] isVector(%d) START", isVector);
        char ackMsg[4], endMsg[4];
        uint vecSize, matInfo[MX_MAT_ATTR_NUM];
        memcpy(ackMsg, "ACK\0", 4);

        if(isVector) {
          debug_print("isVector == TRUE");
          auto &outData( output["out_data"].allocate<std::vector<cv::Mat>>() );
          // 1. Recv num of Matricies
          zmq_recv(sock, &vecSize, sizeof(vecSize), 0);

          for(uint i = 0; i < vecSize; i++) {
            // 2. Recv Mat info, data, and end flag & Allocate data memory
            zmq_recv(sock, matInfo, sizeof(uint)*MX_MAT_ATTR_NUM, 0);
            float *inData = new float[matInfo[MX_MAT_SIZE_IDX]];
            zmq_recv(sock, inData, matInfo[MX_MAT_SIZE_IDX], 0);
            cv::Mat newMat(matInfo[MX_MAT_ROWS_IDX], matInfo[MX_MAT_COLS_IDX], matInfo[MX_MAT_TYPE_IDX], inData);
            outData.push_back(newMat);
            // delete [] inData;
          }
          zmq_recv(sock, endMsg, 4, 0);

          // 3. Send ACK
          zmq_send(sock, ackMsg, 4, 0);

        }
        else {
          debug_print("isVector == FALSE");
          auto &outData( output["out_data"].allocate<cv::Mat>() );
          // 1. Recv num of Matricies
          zmq_recv(sock, &vecSize, sizeof(vecSize), 0);

          // 2. Recv Mat info, data, and end flag & Allocate data memory
          zmq_recv(sock, matInfo, sizeof(uint)*MX_MAT_ATTR_NUM, 0);
          float *inData = new float[matInfo[MX_MAT_SIZE_IDX]];
          zmq_recv(sock, inData, matInfo[MX_MAT_SIZE_IDX], 0);
          zmq_recv(sock, endMsg, 4, 0);
          debug_print("outData init %p, total %d \n", outData.data, outData.total());
          outData = cv::Mat(matInfo[MX_MAT_ROWS_IDX], matInfo[MX_MAT_COLS_IDX], matInfo[MX_MAT_TYPE_IDX], inData);
          debug_print("received %p, matdata %p \n", inData, reinterpret_cast<float*>(outData.data));
          //delete [] inData;
        }

        // 3. Send ACK
        zmq_send(sock, ackMsg, 4, 0);
        output["out_data"].send();
        return raft::proceed;
      }
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif

