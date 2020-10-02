#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

using namespace std;
using namespace cv;

class TestSink : public raft::kernel {
  private:
    bool isVector;
  public:
  TestSink(bool isVector=false): raft::kernel(), isVector(isVector) {
    cout << "TestSink isVector: " << isVector << endl;
    if(isVector)
      input.addPort<vector<mxre::cv_units::Mat>>("in_data");
    else
      input.addPort<mxre::cv_units::Mat>("in_data");
  }

  virtual raft::kstatus run() {
    printf("[TestSink] run \n");
    if(isVector) {
      auto &inData( input["in_data"].template peek<vector<mxre::cv_units::Mat>>() );
      typename vector<mxre::cv_units::Mat>::iterator iter;
      for(iter = inData.begin(); iter != inData.end(); ++iter) {
        printf("\tReceved Vec: %f %f %f %f\n", iter->cvMat.at<float>(0, 0), iter->cvMat.at<float>(0, 1),
            iter->cvMat.at<float>(0, 2), iter->cvMat.at<float>(0, 3));
        iter->release(); // mem leaking
        //delete [] iter->data;
      }
    }
    else {
      auto inData( input["in_data"].template peek<mxre::cv_units::Mat>() );
      printf("\tReceved Vec: %f %f %f %f\n", inData.cvMat.at<float>(0, 0), inData.cvMat.at<float>(0, 1),
          inData.cvMat.at<float>(0, 2), inData.cvMat.at<float>(0, 3));

      // deallocate memory
      debug_print("indata %p \n", inData.data);
      inData.release(); // mem leaking
      //delete [] inData.data;
    }

    input["in_data"].recycle();
    return raft::proceed;
  }
};

int main(int argc, char const *argv[])
{
  cout << "server start!" << endl;
  bool isVector = false;

  TestSink testSink(isVector);
  mxre::pipeline::network::MatReceiver matReceiver(5555, isVector);
  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += matReceiver["out_data"] >> testSink["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}

