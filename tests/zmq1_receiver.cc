#include <bits/stdc++.h>
#include <unistd.h>
#include <zmq.hpp>
#include <opencv2/opencv.hpp>

int main() {
  zmq::context_t ctx(1);
  zmq::socket_t sock(ctx, ZMQ_REP);
  sock.bind("tcp://*:5555");

  zmq::message_t reply(5);
  memcpy(reply.data(), "ack", 3);
  while(true) {
    zmq::message_t env;
    sock.recv(&env);
    std::string env_str = std::string(static_cast<char*>(env.data()), env.size());
    std::cout << "Received envelope '" << env_str << "'" << std::endl;
    //sock.send(reply);

    zmq::message_t msg;
    sock.recv(&msg);
    std::string msg_str = std::string(static_cast<char*>(msg.data()), msg.size());
    std::cout << "Received '" << msg_str << "'" << std::endl;
    sock.send(reply);
  }

  return 0;
}
