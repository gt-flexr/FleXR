#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/video.hpp>
#include <opencv4/opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv) {
  Mat frame;
  VideoCapture cap;
  int deviceID = 2;
  int apiID = cv::CAP_ANY;

  cap.open(deviceID, apiID);
  cap.set(cv::CAP_PROP_FRAME_WIDTH, 320);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, 144);
  cap.set(cv::CAP_PROP_FPS, 60);

  if(!cap.isOpened()) {
    cerr << "ERROR! Unable to open camera\n";
    return -1;
  }

  while(true) {
    auto st = chrono::high_resolution_clock::now();
    cap.read(frame);
    if(frame.empty()) {
      cerr << "ERROR! blank frame grabbed\n";
      break;
    }

    imshow("Live", frame);

    auto et = chrono::high_resolution_clock::now();
    cout << float(chrono::duration_cast<chrono::microseconds>(et-st).count())/1000 << endl;
    if(waitKey(5) >= 0) {
      break;
    }
  }

  return 0;
}
