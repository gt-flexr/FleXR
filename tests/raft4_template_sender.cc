#include <raft>
#include <cstdio>
#include <iostream>
#include <zmq.hpp>

// related thread: https://stackoverflow.com/questions/3786360/confusing-template-error

using namespace std;

template<typename OUT1> // int
class producer: public raft::kernel {
  private:
  int i = 0;

  public:
  producer(): raft::kernel() {
    output.addPort<OUT1>("o1");
  }

  virtual raft::kstatus run() {
    while(i < 1000) {
      output["o1"].push(i++);
      return raft::proceed;
    }

    return raft::stop;
  }
};


template<typename IN1>
class Sender: public raft::kernel {
  private:
    zmq::context_t *ctx;
    zmq::socket_t *sock;

  public:
  Sender(): raft::kernel() {
    input.addPort<IN1>("i1");
    ctx = new zmq::context_t(1);
    sock = new zmq::socket_t(*ctx, ZMQ_REQ);
    sock->connect("tcp://localhost:5555");
    cout << "Sender init done..." << endl;
  }

  ~Sender() {
    delete[] sock;
    delete [] ctx;
  }

  virtual raft::kstatus run() {
    auto in1 = input["i1"].template peek<IN1>();
    printf("[Sender] send %d(%p)\n", in1, &in1);
    zmq::message_t sendMsg(sizeof(IN1));
    zmq::message_t ackMsg;

    memcpy(sendMsg.data(), &in1, sizeof(IN1));
    sock->send(sendMsg);
    sock->recv(ackMsg);
    std::cout << "\tACK: " << static_cast<char*>(ackMsg.data()) << std::endl;

    input["i1"].recycle();
    return raft::proceed;
  }
};

int main() {
  producer<int> a;
  Sender<int> b;

  raft::map m;

  m += a["o1"] >> b["i1"];
  m.exe();

  return EXIT_SUCCESS;
}
