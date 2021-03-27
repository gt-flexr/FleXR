#include <raft>
#include <mxre>
#include <bits/stdc++.h>

using namespace std;

int WIDTH, HEIGHT, QUEUE_SIZE;
std::string absMarkerPath, absImagePath;

int main(int argc, char const *argv[])
{
  zmq::context_t ctx;
  zmq::socket_t subscriber(ctx, zmq::socket_type::sub);

  std::string bindingAddr = "tcp://localhost:5555";
  subscriber.connect(bindingAddr);
  subscriber.set(zmq::sockopt::subscribe, "");


  while(1) {
    std::vector<zmq::message_t> recv_msgs;

    mxre::types::Frame recvFrame;
    recvFrame.data = new unsigned char[1920*1080*3];
    subscriber.recv(zmq::buffer(recvFrame.data, recvFrame.dataSize), zmq::recv_flags::none);

    cv::imshow("Image", recvFrame.useAsCVMat());
    int inKey = cv::waitKey(2) & 0xFF;
    recvFrame.release();
  }
  return 0;
}

