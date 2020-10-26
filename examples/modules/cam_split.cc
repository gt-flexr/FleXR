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
      auto &data(output["out1"].template allocate<frame_with_timestamp>());
      cam.read(data.frame);
      if(data.frame.empty()) {
        cerr << "ERROR: blank frame grabbed" << endl;
        break;
      }
      data.time_stamp = chrono::high_resolution_clock::now();
      output["out1"].send(); // zero copy
      return raft::proceed;
    }

    return raft::stop;
  }
};

class PureCamera: public raft::kernel {
  private:
  VideoCapture cam;
  int frame_idx;

  public:
  PureCamera(): raft::kernel() {
    cam.open(2, cv::CAP_ANY);
    cam.set(CAP_PROP_FRAME_WIDTH, 1280);
    cam.set(CAP_PROP_FRAME_HEIGHT, 720);
    if( !cam.isOpened() ) {
      cerr << "ERROR: unable to open camera" << endl;
    }

    frame_idx = 0;

    output.addPort<Mat>("out1");
  }

  virtual raft::kstatus run() {
    while(frame_idx++ < 1000) {
      auto &data(output["out1"].template allocate<Mat>());
      cam.read(data);
      if(data.empty()) {
        cerr << "ERROR: blank frame grabbed" << endl;
        break;
      }
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

template<typename T>
class tee: public raft::kernel {
  public:
  tee(): raft::kernel() {
    input.addPort<T>("in1");
    output.addPort<T>("out1");
    output.addPort<T>("out2");
  }

  virtual raft::kstatus run() {
    auto test = input["in1"].template peek<T>();

    output["out1"].push(test);
    output["out2"].push(test);
    input["in1"].recycle();
    return raft::proceed;
  }
};

class consumer: public raft::kernel {
  public:
  string name;
  consumer(string name): name(name), raft::kernel() {
    input.addPort<frame_with_timestamp>("in1");
  }

  virtual raft::kstatus run() {
    auto data = input["in1"].peek<frame_with_timestamp>();
    chrono::system_clock::time_point et = chrono::high_resolution_clock::now();
    float et_ms = float(chrono::duration_cast<chrono::microseconds>(
                        et - data.time_stamp).count()) / 1000;
    printf("%s propagation time %f %p \n data: %c %c %c %c %c \n", name.c_str(), et_ms,
        data.frame.data,
        data.frame.at<char>(0, 0),
        data.frame.at<char>(0, 1),
        data.frame.at<char>(0, 2),
        data.frame.at<char>(0, 3),
        data.frame.at<char>(0, 4));
    imshow("test", data.frame);
    input["in1"].recycle();
    if(waitKey(5) >= 0) {
      return raft::stop;
    }
    return raft::proceed;
  }
};

class PureConsumer: public raft::kernel {
  public:
  string name;
  PureConsumer(string name): name(name), raft::kernel() {
    input.addPort<Mat>("in1");
  }

  virtual raft::kstatus run() {
    auto data = input["in1"].peek<Mat>();
    printf("%s data addr %p \n data: %c %c %c %c %c \n", name.c_str(), data.data,
        data.at<char>(0, 0),
        data.at<char>(0, 1),
        data.at<char>(0, 2),
        data.at<char>(0, 3),
        data.at<char>(0, 4));
    imshow("test", data);
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
  PureCamera c;
  PureConsumer cons1("cons1"), cons2("cosn2");
  tee<Mat> t1;

  raft::map m;

  m += c >> t1;
  m += t1["out1"] >> cons1;
  m += t1["out2"] >> cons2;
  m.exe();

  return EXIT_SUCCESS;
}

