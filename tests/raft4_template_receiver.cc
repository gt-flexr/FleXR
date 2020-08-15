#include <cstring>
#include <raft>
#include <cstdio>
#include <iostream>
#include <zmq.hpp>

// related thread: https://stackoverflow.com/questions/3786360/confusing-template-error

using namespace std;

template<typename IN1>
class Sink: public raft::kernel {
  public:
  Sink(): raft::kernel() {
    input.addPort<IN1>("i1");
  }

  virtual raft::kstatus run() {
    auto in1 = input["i1"].template peek<IN1>();
    printf("[Sink] got %d(%p)\n", in1, &in1);
    input["i1"].recycle();
    return raft::proceed;
  }
};


template<typename OUT1>
class Receiver: public raft::kernel {
  private:
    zmq::context_t *ctx;
    zmq::socket_t *sock;

  public:
  Receiver(): raft::kernel() {
    output.addPort<OUT1>("o1");
    ctx = new zmq::context_t(1);
    sock = new zmq::socket_t(*ctx, ZMQ_REP);
    sock->bind("tcp://*:5555");
    cout << "Receiver init done (bound)..." << endl;
  }

  ~Receiver() {
    delete[] sock;
    delete [] ctx;
  }

  virtual raft::kstatus run() {
    zmq::message_t recvMsg;
    zmq::message_t ackMsg(4);
    memcpy(ackMsg.data(), "ACK\0", 4);

    sock->recv(recvMsg);
    OUT1 *recvData = static_cast<OUT1*>(recvMsg.data());

    printf("[Receiver] recv %d(%p)\n", *recvData, recvData);
    sock->send(ackMsg);
    std::cout << "\tACK: " << static_cast<char*>(recvMsg.data()) << std::endl;

    output["o1"].push(*recvData);
    return raft::proceed;
  }
};

template<typename T>
struct Message {
  T *data;
  uint numOfData;
};


template<typename IN1>
class VectorSink: public raft::kernel {
  public:
  VectorSink(): raft::kernel() {
    input.addPort<Message<IN1>>("i1");
  }

  virtual raft::kstatus run() {
    auto &in1( input["i1"].template peek<Message<IN1>>() );
    cout << "[VectorSink] got: ";
    for(int i = 0; i < in1.numOfData; i+=10) {
      cout << in1.data[i] << " ";
    } cout << endl;

    delete [] in1.data;
    input["i1"].recycle();
    return raft::proceed;
  }
};


template<typename OUT1>
class VectorReceiver: public raft::kernel {
  private:
    zmq::context_t *ctx;
    zmq::socket_t *sock;

  public:
  VectorReceiver(): raft::kernel() {
    output.addPort<Message<OUT1>>("o1");
    ctx = new zmq::context_t(1);
    sock = new zmq::socket_t(*ctx, ZMQ_REP);
    sock->bind("tcp://*:5555");
    cout << "Receiver init done (bound)..." << endl;
  }

  ~VectorReceiver() {
    delete[] sock;
    delete [] ctx;
  }

  virtual raft::kstatus run() {
    auto &c( output["o1"].template allocate<Message<OUT1>>() );
    zmq::message_t numMsg, dataMsg, ackMsg(4);
    memcpy(ackMsg.data(), "ACK\0", 4);
    sock->recv(numMsg);
    sock->recv(dataMsg);
    sock->send(ackMsg);
    cout << "RECEIVED!!" << endl;

    c.numOfData = *(static_cast<uint*>(numMsg.data()));
    c.data = new OUT1[c.numOfData];
    memcpy(c.data, dataMsg.data(), sizeof(OUT1) * c.numOfData);

    cout << "[VectorReceiver] got: ";
    for(int i = 0; i < c.numOfData; i+=10) {
      cout << c.data[i] << " ";
    } cout << endl;

    output["o1"].send();
    return raft::proceed;
  }
};



int main() {
  VectorReceiver<int> a;
  VectorSink<int> b;

  raft::map m;

  m += a["o1"] >> b["i1"];
  m.exe();

  return EXIT_SUCCESS;
}
