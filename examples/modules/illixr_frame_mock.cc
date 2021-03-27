#include <bits/stdc++.h>
#include <mxre>
#include <opencv/cv.hpp>
#include <opencv2/highgui.hpp>
#include <unistd.h>

using namespace std;

int main() {
  cv::VideoCapture cap;
  cv::Mat camFrame;
  mxre::types::Frame recvFrame;

  cap.open(0, cv::CAP_ANY);
  cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

  mxre::kernels::ILLIXRSource<mxre::types::Frame> illixrSource;
  mxre::kernels::ILLIXRSink<mxre::types::Frame> illixrSink;

  illixrSource.setup("source", MXRE_DTYPE_FRAME);
  illixrSink.setup("sink", MXRE_DTYPE_FRAME);

  for(int i = 0; i < 1000; i++) {
    cap.read(camFrame);
    mxre::types::Frame sendFrame(camFrame, 0, 0);
    illixrSource.send(&sendFrame);
    illixrSink.recv(&recvFrame);
    cv::imshow("test", recvFrame.useAsCVMat());
    if(cv::waitKey(5) >= 0) {
      break;
    }
    recvFrame.release();
  }
}

