#include <opencv2/opencv.hpp>
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

int main()
{
  zmq::context_t context(1);
  zmq::socket_t sock(context, ZMQ_REQ);
  sock.connect("tcp://localhost:5555");

  zmq::message_t reply;
  cv::Mat sendMat(1, 4, CV_64F);
  sendMat.at<float>(0, 0) = 4;
  sendMat.at<float>(0, 1) = 5;
  sendMat.at<float>(0, 2) = 6;
  sendMat.at<float>(0, 3) = 7;

  for(int n = 0; n < 3; n++) {
    zmq::message_t numMsg(sizeof(int));
    zmq::message_t matMsg(sendMat.elemSize() * sendMat.total());

    int numOfObjs = 4;
    memcpy(numMsg.data(), &numOfObjs, sizeof(numOfObjs));

    sock.send(numMsg, ZMQ_SNDMORE);
    for(int i = 1; i <= numOfObjs; i++) {
      memcpy(matMsg.data(), sendMat.data, matMsg.size());
      if(i == numOfObjs)
        sock.send(matMsg);
      else
        sock.send(matMsg, ZMQ_SNDMORE);
    }

    // recv ACK
    sock.recv(&reply);
    std::cout << "ACK: " << static_cast<char*>(reply.data()) << std::endl;

    sleep(1);
  }
  return 0;
}
