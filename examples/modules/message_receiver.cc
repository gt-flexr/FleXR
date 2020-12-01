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
  bool isVector = false;

  TestSink<int> testSink(isVector);
  //mxre::kernels::MessageReceiver<std::vector<int>> messageReceiver(5555, mxre::utils::recvPrimitiveVector<std::vector<int>>);
  mxre::kernels::MessageReceiver<int> messageReceiver(5555, mxre::utils::recvPrimitive<int>);
  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += messageReceiver["out_data"] >> testSink["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}

