#include <bits/stdc++.h>
#include <mxre>
#include <opencv/cv.hpp>
#include <opencv2/highgui.hpp>
#include <unistd.h>

using namespace std;

int main() {
  cv::VideoCapture cap;
  cv::Mat sendFrame, recvFrame;

  cap.open(2, cv::CAP_ANY);
  cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 144);

  mxre::types::ILLIXRSink<cv::Mat> illixrSink;
  mxre::types::ILLIXRSource<cv::Mat> illixrSource;

  illixrSource.setup("source", MX_DTYPE_CVMAT);
  illixrSink.setup("sink", MX_DTYPE_CVMAT);

  int j;
  for(int i = 0; i < 100; i++) {
    cap.read(sendFrame);
    illixrSource.send(&sendFrame);
    illixrSink.recv(&recvFrame);
    cv::imshow("test", recvFrame);
    if(cv::waitKey(5) >= 0) {
      break;
    }
  }
}

