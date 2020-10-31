#include <raft>
#include <mxre>
#include <bits/stdc++.h>

using namespace std;
using namespace cv;

template<typename IN_T>
class TestSink : public raft::kernel {
  private:
    bool isVector;
  public:
  TestSink(bool isVector=false): raft::kernel(), isVector(isVector) {
    cout << "TestSink isVector: " << isVector << endl;
    if(isVector)
      input.addPort<vector<IN_T>>("in_data");
    else
      input.addPort<IN_T>("in_data");
  }

  virtual raft::kstatus run() {
    printf("[TestSink] run \n");
    if(isVector) {
      auto &inData( input["in_data"].template peek<vector<IN_T>>() );
      typename vector<IN_T>::iterator iter;
      for(iter = inData.begin(); iter != inData.end(); ++iter) {
        cout << *iter << " ";
      }
      cout << endl;
    }
    else {
      auto &inData( input["in_data"].template peek<IN_T>() );
      cout << "received: " << inData << endl;
    }

    input["in_data"].recycle();
    return raft::proceed;
  }
};

int main(int argc, char const *argv[])
{
  cout << "server start!" << endl;
  bool isVector = true;

  TestSink<int> testSink(isVector);
  mxre::kernels::StaticReceiver<int> staticReceiver(5555, isVector);
  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += staticReceiver["out_data"] >> testSink["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}
