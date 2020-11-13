#include <raft>
#include <mxre>
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
      input.addPort<vector<cv::Mat>>("in_data");
    else
      input.addPort<cv::Mat>("in_data");
  }

  virtual raft::kstatus run() {
    printf("[TestSink] run \n");
    if(isVector) {
      auto &inData( input["in_data"].template peek<vector<cv::Mat>>() );
      typename vector<cv::Mat>::iterator iter;
      for(iter = inData.begin(); iter != inData.end(); ++iter) {
        printf("\tReceved Vec: %f %f %f %f\n", iter->at<float>(0, 0), iter->at<float>(0, 1),
            iter->at<float>(0, 2), iter->at<float>(0, 3));
        iter->release(); // mem leaking
        //delete [] iter->data;
      }
    }
    else {
      auto inData( input["in_data"].template peek<cv::Mat>() );
      printf("\tReceved Vec: %f %f %f %f\n", inData.at<float>(0, 0), inData.at<float>(0, 1),
          inData.at<float>(0, 2), inData.at<float>(0, 3));

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
  mxre::kernels::MatReceiver matReceiver(5555, isVector);
  cout << "created pipeline elements" << endl;

  raft::map pipeline;

  pipeline += matReceiver["out_data"] >> testSink["in_data"];
  cout << "\tpipeline.exe" << endl;
  pipeline.exe();
  return 0;
}
