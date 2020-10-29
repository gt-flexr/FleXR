#ifndef __MXRE_CVMAT_RECEIVER__
#define __MXRE_CVMAT_RECEIVER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <cv.hpp>

#include "defs.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "kernels/offloading/message_receiver.h"

namespace mxre
{
  namespace kernels
  {

    /* Class Deifinition */
    class MatReceiver : public MessageReceiver
    {
      private:
        bool isVector;
      public:
        MatReceiver(int port=5555, bool isVector=false);
        virtual raft::kstatus run();
    };


    /* Constructor() */
    MatReceiver::MatReceiver(int port, bool isVector) : MessageReceiver(port) {
      this->isVector = isVector;
      if(isVector)
        output.addPort<std::vector<mxre::cv_types::Mat>>("out_data");
      else
        output.addPort<mxre::cv_types::Mat>("out_data");
    }


    /* Run() */
    raft::kstatus MatReceiver::run() {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      debug_print("[MatReceiver] isVector(%d) START", isVector);
      char ackMsg[4], endMsg[4];
      uint vecSize, matInfo[MX_MAT_ATTR_NUM];
      memcpy(ackMsg, "ACK\0", 4);

      if(isVector) {
        auto &outData( output["out_data"].allocate<std::vector<mxre::cv_types::Mat>>() );
        // 1. Recv num of Matricies
        zmq_recv(sock, &vecSize, sizeof(vecSize), 0);

        for(uint i = 0; i < vecSize; i++) {
          // 2. Recv Mat info, data, and end flag & Allocate data memory
          zmq_recv(sock, matInfo, sizeof(uint)*MX_MAT_ATTR_NUM, 0);

          void *inData;
          if(matInfo[MX_MAT_TYPE_IDX] == CV_32F)
            inData = new float[matInfo[MX_MAT_SIZE_IDX]];
          else {
            debug_print("Not handled matrix type!!");
            exit(1);
          }

          zmq_recv(sock, inData, matInfo[MX_MAT_SIZE_IDX], 0);
          mxre::cv_types::Mat newMat(matInfo[MX_MAT_ROWS_IDX], matInfo[MX_MAT_COLS_IDX], matInfo[MX_MAT_TYPE_IDX], inData);
          outData.push_back(newMat);
          // delete [] inData;
        }
        zmq_recv(sock, endMsg, 4, 0);

        // 3. Send ACK
        zmq_send(sock, ackMsg, 4, 0);
      }
      else {
        debug_print("isVector == FALSE");
        auto &outData( output["out_data"].allocate<mxre::cv_types::Mat>() );
        // 1. Recv num of Matricies
        zmq_recv(sock, &vecSize, sizeof(vecSize), 0);

        // 2. Recv Mat info, data, and end flag & Allocate data memory
        zmq_recv(sock, matInfo, sizeof(uint)*MX_MAT_ATTR_NUM, 0);

        void *inData;
        if(matInfo[MX_MAT_TYPE_IDX] == CV_32F)
          inData = new float[matInfo[MX_MAT_SIZE_IDX]];
        else
          debug_print("Not handled matrix type!!");

        zmq_recv(sock, inData, matInfo[MX_MAT_SIZE_IDX], 0);
        zmq_recv(sock, endMsg, 4, 0);
        debug_print("outData init %p, total %d \n", outData.data, outData.total);
        outData = mxre::cv_types::Mat(matInfo[MX_MAT_ROWS_IDX], matInfo[MX_MAT_COLS_IDX], matInfo[MX_MAT_TYPE_IDX], inData);
        debug_print("received %p, matdata %p \n", inData, reinterpret_cast<float*>(outData.data));
        //delete [] inData;
      }

      // 3. Send ACK
      zmq_send(sock, ackMsg, 4, 0);

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      output["out_data"].send();
      return raft::proceed;
    }

  }   // namespace kernels
} // namespace mxre

#endif

