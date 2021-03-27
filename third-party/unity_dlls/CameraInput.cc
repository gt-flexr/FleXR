#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <bits/stdc++.h>

#define DllExport __attribute__((visibility("default")))

using namespace std;
using namespace cv;

String _windowName = "Unity OpenCV Interop Sample";
VideoCapture _capture;
int _scale = 1;

struct Color32
{
	uchar red, green, blue, alpha;
};

extern "C" {
  DllExport int Init(int& outCamWidth, int& outCamHeight) {
    _capture.open(0);
    if(!_capture.isOpened()) return -1;

    int width = 1280, height = 720;

    _capture.set(CAP_PROP_FRAME_WIDTH, width);
    _capture.set(CAP_PROP_FRAME_HEIGHT, height);

    outCamWidth = width;
    outCamHeight = height;

    return 0;
  }

  DllExport void Close() { _capture.release(); }

  DllExport int GetFrame(Color32 **frameBuffer) {
    Mat frame;
    _capture.read(frame);
    if(frame.empty()) return -1;

    flip(frame, frame, 0);
    Mat UnityFrame(frame.rows, frame.cols, CV_8UC4, *frameBuffer);
    cvtColor(frame, UnityFrame, COLOR_RGB2BGRA);

    return 1;
  }
}

