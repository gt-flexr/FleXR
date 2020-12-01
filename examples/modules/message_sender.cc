#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <unistd.h>

// related post
// https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

using namespace std;
using namespace cv;

template<typename OUT_T>
class TestSrc : public raft::kernel {
  private:
    bool isVector;
    int numMat;
    int data;
  public:
  TestSrc(bool isVector=false, int numMat=1): raft::kernel(), isVector(isVector), numMat(numMat) {
    cout << "TestSrc isVector: " << isVector << endl;
    data = 0;
    if(isVector)
      output.addPort<vector<OUT_T>>("out_data");
    else
      output.addPort<OUT_T>("out_data");
  }

  virtual raft::kstatus run() {
    printf("[TestSource] run \n");
    if(isVector) {
      auto &outData( output["out_data"].template allocate<vector<OUT_T>>() );
      for(int i = 0; i < numMat; i++) {
        OUT_T newElem = i;
        outData.push_back(newElem);
      }
    }
    else {
      auto &outData( output["out_data"].template allocate<OUT_T>() );
      outData = data++;
    }
    //sleep(1);
    output["out_data"].send();
    return raft::proceed;
  }
};

int main(int argc, char const *argv[])
{
  cout << "client start!" << endl;
  bool isVector = false;

  TestSrc<int> testSrc(isVector, 8192);
  //mxre::kernels::MessageSender<std::vector<int>> messageSender("localhost", 5555, mxre::utils::sendPrimitiveVector<std::vector<int>>);
  mxre::kernels::MessageSender<int> messageSender("localhost", 5555, mxre::utils::sendPrimitive<int>);
  //mxre::kernels::MessageSender<int> messageSender("localhost", 5555, mxre::utils::sendPrimitive<int>);

  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += testSrc["out_data"] >> messageSender["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}

