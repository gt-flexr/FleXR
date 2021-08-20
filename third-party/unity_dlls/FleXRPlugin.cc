#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <bits/stdc++.h>

#define DllExport __attribute__((visibility("default")))

using namespace std;
using namespace cv;

zmq::context_t ctx;
zmq::socket_t subscriber;
int width, height;
bool isReady;

struct Color32
{
	uchar red, green, blue, alpha;
};


extern "C" {
  DllExport int Init(int inID, int inFrameWidth, int inFrameHeight) {
    std::string bindingAddr = "tcp://localhost:" + std::to_string(inID);

    subscriber =  zmq::socket_t(ctx, zmq::socket_type::sub);
    subscriber.connect(bindingAddr);
    subscriber.set(zmq::sockopt::subscribe, "");

    width = inFrameWidth;
    height = inFrameHeight;
    isReady = true;

    return 0;
  }

  DllExport void Close() {
    subscriber.close();
    ctx.shutdown();
    ctx.close();
  }


  DllExport int GetFrame(Color32 **frameBuffer) {
    if(!isReady) return -1;
    unsigned char *recvBuffer = new unsigned char[width*height*3];
    subscriber.recv(zmq::buffer(recvBuffer, width*height*3));

    cv::Mat frame(height, width, CV_8UC3, recvBuffer);
    flip(frame, frame, 0);

    Mat UnityFrame(height, width, CV_8UC4, *frameBuffer);
    cvtColor(frame, UnityFrame, COLOR_RGB2BGRA);

    delete [] recvBuffer;
    return 1;
  }
}

