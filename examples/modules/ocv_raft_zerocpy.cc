#include <raft>
#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include <unistd.h>
#include <termios.h>

char getch(void)
{
  char buf = 0;
  struct termios old = {0};
  fflush(stdout);
  if(tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if(tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if(read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if(tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return buf;
}



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
    while(frame_idx++ < 2000) {
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


class Keyboard: public raft::kernel {
  public:
  char in;
  Keyboard(): raft::kernel() {
    output.addPort<char>("out1");
  }

  virtual raft::kstatus run() {
    auto data(output["out1"].template allocate_s<char>());

    // 1. key input
    (*data) = getch();
    cout << "send: " << (*data) << endl;
    return raft::proceed;
  }
};

class consumer: public raft::kernel {
  public:
  consumer(): raft::kernel() {
    input.addPort<frame_with_timestamp>("in1");
    input.addPort<char>("in2");
  }

  virtual raft::kstatus run() {
    auto data = input["in1"].peek<frame_with_timestamp>();

    auto &keyPort( input["in2"] );
    if( keyPort.size() > 0 ) {
      auto key = input["in2"].peek<char>();
      cout << "In Key: " << key << endl;
      keyPort.recycle(1);
    }

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
  Keyboard kb;
  consumer cons;

  raft::map m;

  m += c >> cons["in1"];
  m += kb >> cons["in2"];
  m.exe();

  return EXIT_SUCCESS;
}

