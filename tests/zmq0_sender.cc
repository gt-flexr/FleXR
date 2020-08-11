#include <bits/stdc++.h>
#include <unistd.h>
#include <zmq.hpp>
#include <opencv2/opencv.hpp>

int main() {
  zmq::context_t ctx(1);
  zmq::socket_t sock(ctx, ZMQ_REQ);
  sock.connect("tcp://localhost:5555");

  cv::Mat sendMat(1, 4, CV_64F);
  sendMat.at<float>(0, 0) = 1;
  sendMat.at<float>(0, 1) = 2;
  sendMat.at<float>(0, 2) = 3;
  sendMat.at<float>(0, 3) = 4;
  std::cout << sendMat.size() << std::endl;
  std::cout << sendMat.elemSize() * sendMat.total() << std::endl;
  std::cout << sendMat.at<float>(0, 3) << std::endl;

  for(int i = 0; i < 10; i ++) {
    zmq::message_t request(sendMat.elemSize() * sendMat.total());
    memcpy(request.data(), sendMat.data, sendMat.elemSize() * sendMat.total());

    float *test = static_cast<float*>(request.data());
    std::cout << test[0] << std::endl;
    std::cout << test[1] << std::endl;
    std::cout << test[2] << std::endl;
    std::cout << test[3] << std::endl;

    sock.send(request);

    zmq::message_t reply;
    sock.recv(&reply);
    std::cout << "ACK: " << static_cast<char*>(reply.data()) << std::endl;
  }

  return 0;
}
