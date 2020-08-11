#include <bits/stdc++.h>
#include <unistd.h>
#include <zmq.hpp>
#include <opencv2/opencv.hpp>

int main() {
  zmq::context_t ctx(1);
  zmq::socket_t sock(ctx, ZMQ_REP);
  sock.bind("tcp://*:5555");

  while(true) {
    zmq::message_t request;
    sock.recv(&request);

    float *test = static_cast<float*>(request.data());
    std::cout << test[0] << std::endl;
    std::cout << test[1] << std::endl;
    std::cout << test[2] << std::endl;
    std::cout << test[3] << std::endl;

    std::cout << "====================" << std::endl;
    cv::Mat recvMat(1, 4, CV_64F, test);
    std::cout << recvMat.at<float>(0, 0) << std::endl;
    std::cout << recvMat.at<float>(0, 1) << std::endl;
    std::cout << recvMat.at<float>(0, 2) << std::endl;
    std::cout << recvMat.at<float>(0, 3) << std::endl;
    sleep(1);

    zmq::message_t reply(5);
    memcpy(reply.data(), "ack", 3);
    sock.send(reply);
  }

  return 0;
}
