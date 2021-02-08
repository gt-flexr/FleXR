#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <bits/stdc++.h>

#define DllExport __attribute__((visibility("default")))

using namespace std;
using namespace cv;

struct Circle {
  Circle(int x, int y, int radius) : X(x), Y(y), Radius(radius) {}
  int X, Y, Radius;
};

CascadeClassifier _faceCascade;
String _windowName = "Unity OpenCV Interop Sample";
VideoCapture _capture;
int _scale = 1;

extern "C" {
  DllExport int Init(int& outCamWidth, int& outCamHeight) {
    if(!_faceCascade.load("/home/jin/Unity/Projects/Unity_OpenCV_Tutorial/Assets/lbpcascade_frontalface.xml")) return -1;

    _capture.open(0);
    if(!_capture.isOpened()) return -2;

    int width = 1280, height = 720;

    _capture.set(CAP_PROP_FRAME_WIDTH, width);
    _capture.set(CAP_PROP_FRAME_HEIGHT, height);

    outCamWidth = width;
    outCamHeight = height;

    return 0;
  }

  DllExport void Close() { _capture.release(); }
  DllExport void SetScale(int scale) { _scale = scale; }
  DllExport void Detect(Circle* outFaces, int maxOutFacesCount, int& outDetectedFacesCount) {
    Mat frame;
    _capture >> frame;
    if(frame.empty()) return;

    std::vector<Rect> faces;
    Mat grayscaleFrame, resizedGray;
    cvtColor(frame, grayscaleFrame, COLOR_BGR2GRAY);

    resize(grayscaleFrame, resizedGray, Size(frame.cols / _scale, frame.rows / _scale));
    equalizeHist(resizedGray, resizedGray);

    _faceCascade.detectMultiScale(resizedGray, faces);

    for(size_t i = 0; i < faces.size(); i++) {
      Point center(_scale * (faces[i].x+faces[i].width /2), _scale * (faces[i].y+faces[i].height /2));
      ellipse(frame, center, Size(_scale * faces[i].width / 2, _scale * faces[i].height / 2),
              0, 0, 360, Scalar(0, 0, 255), 4, 8, 0);

      outFaces[i] = Circle(faces[i].x, faces[i].y, faces[i].width/2);
      outDetectedFacesCount++;

      if(outDetectedFacesCount == maxOutFacesCount) break;
    }

    imshow(_windowName, frame);
  }
}

