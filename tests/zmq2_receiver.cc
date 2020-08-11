#include <bits/stdc++.h>
#include <unistd.h>
#include <zmq.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

int main() {
  zmq::context_t ctx(1);
  zmq::socket_t sock(ctx, ZMQ_REP);
  sock.bind("tcp://*:5555");

  zmq::message_t numMsg, matMsg;
  zmq::message_t reply(5);
  memcpy(reply.data(), "ack", 3);
  while(true) {
    sock.recv(&numMsg);
    int *numOfObjs = static_cast<int*>(numMsg.data());
    cout << "numOfObjs" << *numOfObjs << endl;

    for(int i = 0; i < *numOfObjs; i++) {
      sock.recv(&matMsg);
      float *matData = static_cast<float*>(matMsg.data());
      printf("Received Data: %f %f %f %f \n", matData[0], matData[1], matData[2], matData[3]);
      cv::Mat recvMat(1, 4, CV_64F, matData);
      printf("Created Mat: %f %f %f %f \n", recvMat.at<float>(0, 0), recvMat.at<float>(0, 1),
          recvMat.at<float>(0, 2), recvMat.at<float>(0, 3));
      cout << "=====================================" << endl;
    }
    // send ACK
    sock.send(reply);
  }

  return 0;
}
