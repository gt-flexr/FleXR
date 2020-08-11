#include <zmq.hpp>
#include <string>
#include <iostream>
#include <bits/stdc++.h>
#include <unistd.h>

int main()
{
  zmq::context_t context(1);
  zmq::socket_t sock(context, ZMQ_REQ);
  sock.connect("tcp://localhost:5555");

  zmq::message_t reply;

  for(int n = 0; n < 3; n++) {
    zmq::message_t env1(1);
    memcpy(env1.data(), "A", 1);
    std::string msg1_str = "[A] msg1_str";

    zmq::message_t msg1(msg1_str.size());
    memcpy(msg1.data(), msg1_str.c_str(), msg1_str.size());

    std::cout << "Sending '" << msg1_str << "' on topic A" << std::endl;
    sock.send(env1, ZMQ_SNDMORE);
    sock.send(msg1);
    // recv ACK
    sock.recv(&reply);
    std::cout << "ACK: " << static_cast<char*>(reply.data()) << std::endl;


    zmq::message_t env2(1);
    memcpy(env2.data(), "B", 1);
    std::string msg2_str = "[B] msg2_str";

    zmq::message_t msg2(msg2_str.size());
    memcpy(msg2.data(), msg2_str.c_str(), msg2_str.size());

    std::cout << "Sending '" << msg2_str << "' on topic B" << std::endl;
    sock.send(env2, ZMQ_SNDMORE);
    sock.send(msg2);

    sock.recv(&reply);
    std::cout << "ACK: " << static_cast<char*>(reply.data()) << std::endl;

    sleep(1);
  }
  return 0;
}
