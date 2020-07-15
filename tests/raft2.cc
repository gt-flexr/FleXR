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
    output.addPort<int>("o2");
  }

  virtual raft::kstatus run() {
    while(i < 1000) {
      output["o1"].push(i);
      output["o2"].push(i++);
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
    input.addPort<int>("i2");
  }

  virtual raft::kstatus run() {
    auto in1 = input["i1"].peek<int>();
    auto in2 = input["i2"].peek<int>();
    printf("[Consumer] got %d(%p), %d(%p) \n", in1, &in1, in2, &in2);
    input["i1"].recycle();
    input["i2"].recycle();
    return raft::proceed;
  }
};

int main() {
  producer a;
  processor b1;
  processor b2;
  consumer c;

  raft::map m;

  m += a["o1"] >> b1["i1"];
  m += a["o2"] >> b2["i1"];
  m += b1 >> c["i1"];
  m += b2 >> c["i2"];
  m.exe();

  return EXIT_SUCCESS;
}