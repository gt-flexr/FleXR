#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>
#include <unistd.h>

// related post
// https://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

using namespace std;
using namespace cv;

class TestSrc : public raft::kernel {
  private:
    bool isVector;
    int numMat;
  public:
  TestSrc(bool isVector=false, int numMat=1): raft::kernel(), isVector(isVector), numMat(numMat) {
    cout << "TestSrc isVector: " << isVector << endl;
    if(isVector)
      output.addPort<vector<Mat>>("out_data");
    else
      output.addPort<Mat>("out_data");
  }

  virtual raft::kstatus run() {
    printf("[TestSource] run \n");
    if(isVector) {
      auto &outData( output["out_data"].template allocate<vector<Mat>>() );
      for(int i = 0; i < numMat; i++) {
        Mat newMat(16, 2048, CV_32F);
        newMat.at<float>(0, 0) = i;
        newMat.at<float>(0, 1) = i+1;
        newMat.at<float>(0, 2) = i+2;
        newMat.at<float>(0, 3) = i+3;
        outData.push_back(newMat);
      }
    }
    else {
      auto &outData( output["out_data"].template allocate<Mat>() );
      outData = Mat(16, 2048, CV_32F);
      outData.at<float>(0, 0) = 0;
      outData.at<float>(0, 1) = 1;
      outData.at<float>(0, 2) = 2;
      outData.at<float>(0, 3) = 3;
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

  TestSrc testSrc(isVector, 5);
  mxre::pipeline::network::CVMatSender matSender("localhost", 5555, isVector);
  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += testSrc["out_data"] >> matSender["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}
