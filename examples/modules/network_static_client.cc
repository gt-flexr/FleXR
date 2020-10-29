#include <raft>
#include <mxre.h>
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
  public:
  TestSrc(bool isVector=false, int numMat=1): raft::kernel(), isVector(isVector), numMat(numMat) {
    cout << "TestSrc isVector: " << isVector << endl;
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
      outData = 4;
    }
    //sleep(1);
    output["out_data"].send();
    return raft::proceed;
  }
};

int main(int argc, char const *argv[])
{
  cout << "client start!" << endl;
  bool isVector = true;

  TestSrc<int> testSrc(isVector, 8192);
  mxre::kernels::StaticSender<int> staticSender("localhost", 5555, isVector);
  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += testSrc["out_data"] >> staticSender["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}
