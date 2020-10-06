#ifndef __MXRE_CVMAT_SENDER__
#define __MXRE_CVMAT_SENDER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <cv.hpp>
#include "defs.h"
#include "message_sender.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      /* Class Deifinition */
      class MatSender : public MessageSender
      {
        private:
          bool isVector;
        public:
          MatSender(std::string addr="localhost", int port=5555, bool isVector=false);
          virtual raft::kstatus run();
      };


      /* Constructor() */
      MatSender::MatSender(std::string addr, int port, bool isVector) :
          MessageSender(addr, port) {
        this->isVector = isVector;
        if(isVector)
          input.addPort<std::vector<mxre::cv_units::Mat>>("in_data");
        else
          input.addPort<mxre::cv_units::Mat>("in_data");
      }


      /* Run() */
      raft::kstatus MatSender::run() {

#ifdef __PROFILE__
        TimeVal start = getNow();
#endif
        char ackMsg[4];
        std::vector<mxre::cv_units::Mat> inMat;
        if(isVector) {
          inMat = input["in_data"].template peek<std::vector<mxre::cv_units::Mat>>();
        }
        else {
          auto &inData( input["in_data"].template peek<mxre::cv_units::Mat>() );
          inMat.push_back(inData);
        }

        // 1. Send num of Matrices
        uint vecSize = inMat.size();
        zmq_send(sock, &vecSize, sizeof(uint), ZMQ_SNDMORE);

        // 2. Send each Mat info & data (iteration) & end flag
        for(uint i = 0; i < inMat.size(); i++) {
          uint matInfo[MX_MAT_ATTR_NUM];
          matInfo[MX_MAT_SIZE_IDX] = inMat[i].total * inMat[i].elemSize;
          matInfo[MX_MAT_ROWS_IDX] = inMat[i].rows;
          matInfo[MX_MAT_COLS_IDX] = inMat[i].cols;
          matInfo[MX_MAT_TYPE_IDX] = inMat[i].type;
          zmq_send(sock, matInfo, sizeof(uint)*MX_MAT_ATTR_NUM, ZMQ_SNDMORE);
          zmq_send(sock, inMat[i].data, matInfo[0], ZMQ_SNDMORE);
        }
        zmq_send(sock, "end\0", 4, 0);

        // 3. Recv ACK
        zmq_recv(sock, ackMsg, 4, 0);

        // 4. Deallocate mat data
        for(uint i = 0; i < inMat.size(); i++) {
          inMat[i].release();
        }
        input["in_data"].recycle(1);

#ifdef __PROFILE__
        TimeVal end = getNow();
        profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

        return raft::proceed;
      }
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif

