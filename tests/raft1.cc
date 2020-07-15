#include <raft>
#include <cstdio>
#include <iostream>

using namespace std;

class producer: public raft::kernel {
  private:
  int i = 0;

  public:
  producer(): raft::kernel() {
    output.addPort<int>("o1");
  }

  virtual raft::kstatus run() {
    while(i < 1000) {
      output["o1"].push(i++);
      return raft::proceed;
    }

    return raft::stop;
  }
};

class processor: public raft::kernel {
  public:
  processor(): raft::kernel() {
    input.addPort<int>("i1");
    output.addPort<int>("o1");
  }

  virtual raft::kstatus run() {
    output["o1"].push( input["i1"].peek<int>() );
    input["i1"].recycle();
    return raft::proceed;
  }
};

class consumer: public raft::kernel {
  public:
  consumer(): raft::kernel() {
    input.addPort<int>("i1");
  }

  virtual raft::kstatus run() {
    auto in1 = input["i1"].peek<int>();
    printf("[Consumer] got %d (%p) \n", in1, &in1);
    input["i1"].recycle();
    return raft::proceed;
  }
};

int main() {
  producer a;
  processor b;
  consumer c;

  raft::map m;
  m += a >> b >> c;
  m.exe();

  return EXIT_SUCCESS;
}