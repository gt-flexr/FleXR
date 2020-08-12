#include <raft>
#include <cstdio>
#include <iostream>

// related thread: https://stackoverflow.com/questions/3786360/confusing-template-error

using namespace std;

template<typename OUT1, typename OUT2> // int
class producer: public raft::kernel {
  private:
  int i = 0;

  public:
  producer(): raft::kernel() {
    output.addPort<OUT1>("o1");
    output.addPort<OUT2>("o2");
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


template<typename IN, typename OUT>
class processor: public raft::kernel {
  public:
  processor(): raft::kernel() {
    input.addPort<IN>("i1");
    output.addPort<OUT>("o1");
  }

  virtual raft::kstatus run() {
    output["o1"].push( input["i1"].template peek<IN>() );
    input["i1"].recycle();
    return raft::proceed;
  }
};

template<typename IN1, typename IN2>
class consumer: public raft::kernel {
  public:
  consumer(): raft::kernel() {
    input.addPort<IN1>("i1");
    input.addPort<IN2>("i2");
  }

  virtual raft::kstatus run() {
    auto in1 = input["i1"].template peek<IN1>();
    auto in2 = input["i2"].template peek<IN2>();
    printf("[Consumer] got %d(%p), %d(%p) \n", in1, &in1, in2, &in2);
    input["i1"].recycle();
    input["i2"].recycle();
    return raft::proceed;
  }
};

int main() {
  producer<int, int> a;
  processor<int, int> b1;
  processor<int, int> b2;
  consumer<int, int> c;

  raft::map m;

  m += a["o1"] >> b1["i1"];
  m += a["o2"] >> b2["i1"];
  m += b1 >> c["i1"];
  m += b2 >> c["i2"];
  m.exe();

  return EXIT_SUCCESS;
}
