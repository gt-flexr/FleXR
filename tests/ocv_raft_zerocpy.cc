#include <raft>
//#include <cstdio>
//#include <iostream>
//#include <chrono>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

typedef struct frame_with_timestamp {
  Mat frame;
  chrono::system_clock::time_point time_stamp;
} frame_with_timestamp;

class Camera: public raft::kernel {
  private:
  VideoCapture cam;
  int frame_idx;

  public:
  Camera(): raft::kernel() {
    cam.open(2, cv::CAP_ANY);
    cam.set(CAP_PROP_FRAME_WIDTH, 1280);
    cam.set(CAP_PROP_FRAME_HEIGHT, 720);
    if( !cam.isOpened() ) {
      cerr << "ERROR: unable to open camera" << endl;
    }

    frame_idx = 0;

    output.addPort<frame_with_timestamp>("out1");
  }

  virtual raft::kstatus run() {
    while(frame_idx++ < 1000) {
      auto data(output["out1"].template allocate_s<frame_with_timestamp>());
      cam.read((*data).frame);
      if((*data).frame.empty()) {
        cerr << "ERROR: blank frame grabbed" << endl;
        break;
      }
      (*data).time_stamp = chrono::high_resolution_clock::now();
      output["out1"].send(); // zero copy
      return raft::proceed;
    }

    return raft::stop;
  }
};

class processor: public raft::kernel {
  public:
  processor(): raft::kernel() {
    input.addPort<frame_with_timestamp>("in1");
    output.addPort<frame_with_timestamp>("out1");
  }

  virtual raft::kstatus run() {
    auto test = input["in1"].peek<frame_with_timestamp>();

    output["out1"].template allocate_s<frame_with_timestamp>(test);
    output["out1"].send();

    input["in1"].recycle();
    return raft::proceed;
  }
};

class consumer: public raft::kernel {
  public:
  consumer(): raft::kernel() {
    input.addPort<frame_with_timestamp>("in1");
  }

  virtual raft::kstatus run() {
    auto data = input["in1"].peek<frame_with_timestamp>();
    chrono::system_clock::time_point et = chrono::high_resolution_clock::now();
    float et_ms = float(chrono::duration_cast<chrono::microseconds>(
                        et - data.time_stamp).count()) / 1000;
    cout << "propagation time: " << et_ms << endl;
    imshow("Test", data.frame);
    input["in1"].recycle();
    if(waitKey(5) >= 0) {
      return raft::stop;
    }
    return raft::proceed;
  }
};

int main(int argc, char **argv) {
  // Tell GLUT that whenever the main window needs to be repainted that it
  // should call the function display().
  Camera c;
  processor b1;
  processor b2;
  processor b3;
  processor b4;
  processor b5;
  processor b6;
  consumer cons;

  raft::map m;

  m += c >> b1 >> b2 >> b3 >> b4 >> b5 >> b6 >> cons;
  m.exe();

  return EXIT_SUCCESS;
}