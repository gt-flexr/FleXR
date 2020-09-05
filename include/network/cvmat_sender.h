#ifndef __MXRE_CVMAT_SENDER__
#define __MXRE_CVMAT_SENDER__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <cv.hpp>
#include "defs.h"
#include "message_sender.h"

namespace mxre
{
  namespace pipeline
  {
    namespace network
    {
      /* Class Deifinition */
      class CVMatSender : public MessageSender
      {
        private:
          bool isVector;
        public:
          CVMatSender(std::string addr="localhost", int port=5555, bool isVector=false);
          virtual raft::kstatus run();
      };


      /* Constructor() */
      CVMatSender::CVMatSender(std::string addr, int port, bool isVector) :
          MessageSender(addr, port) {
        this->isVector = isVector;
        if(isVector)
          input.addPort<std::vector<cv::Mat>>("in_data");
        else
          input.addPort<cv::Mat>("in_data");
      }


      /* Run() */
      raft::kstatus CVMatSender::run() {
        debug_print("[CVMatSender] isVector(%d) START", isVector);
        char ackMsg[4];
        std::vector<cv::Mat> inMat;
        if(isVector) {
          //auto &inData( input["in_data"].template peek<std::vector<cv::Mat>>() );
          inMat = input["in_data"].template peek<std::vector<cv::Mat>>();
        }
        else {
          auto &inData( input["in_data"].template peek<cv::Mat>() );
          inMat.push_back(inData);
        }

        // 1. Send num of Matrices
        uint vecSize = inMat.size();
        zmq_send(sock, &vecSize, sizeof(uint), ZMQ_SNDMORE);

        // 2. Send each Mat info & data (iteration) & end flag
        for(uint i = 0; i < inMat.size(); i++) {
          uint matInfo[MX_MAT_ATTR_NUM];
          matInfo[MX_MAT_SIZE_IDX] = inMat[i].total() * inMat[i].elemSize();
          matInfo[MX_MAT_ROWS_IDX] = inMat[i].rows;
          matInfo[MX_MAT_COLS_IDX] = inMat[i].cols;
          zmq_send(sock, matInfo, sizeof(uint)*MX_MAT_ATTR_NUM, ZMQ_SNDMORE);
          zmq_send(sock, inMat[i].data, matInfo[0], ZMQ_SNDMORE);
        }
        zmq_send(sock, "end\0", 4, 0);

        // 3. Recv ACK
        zmq_recv(sock, ackMsg, 4, 0);

        // 4. Deallocate mat data
        for(uint i = 0; i < inMat.size(); i++) {
          inMat[i].release();
          if(inMat[i].data) delete [] inMat[i].data; // for externally allocated data
        }
        input["in_data"].recycle(1);
        return raft::proceed;
      }
    } // namespace network
  }   // namespace pipeline
} // namespace mxre

#endif

