#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

using namespace std;

class TestSink : public raft::kernel {
  public:
  TestSink(): raft::kernel() {
    input.addPort<cv::Mat>("in_data");
  }

  virtual raft::kstatus run() {
    auto &inData( input["in_data"].template peek<cv::Mat>() );
    cv::Mat test = inData.clone();


    if(inData.rows == HEIGHT && inData.cols == WIDTH) {
      debug_print("CVMat data address: %p", inData.data);

      cv::imshow("press ESC to exit", test);
      if(cv::waitKey(100) == 0x1b) {
        std::cout << "key down" << std::endl;
      }
    }

    //if(inData.data) delete [] inData.data;
    input["in_data"].recycle();
    return raft::proceed;
  }
};



int main(int argc, char const *argv[])
{
  mxre::pipeline::network::RTPFrameReceiver rtpReceiver("mjpeg", WIDTH, HEIGHT);
  TestSink testSink;
  raft::map pipeline;

  // cam - obj detector
  pipeline += rtpReceiver["out_data"] >> testSink["in_data"];

  pipeline.exe();
  return 0;
}

