#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <vector>
#include <queue>
#include <limits>
#include <string>
#include <sys/time.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>

using namespace cv;
using namespace std;
using timmeval=struct timeval;

#define WIDTH 1920
#define HEIGHT 1080

int main(int argc, char **argv)
{
  VideoCapture cap;
  int deviceID = 2;
  int apiID = cv::CAP_ANY;

  cap.open(deviceID, apiID);
  cap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
  cap.set(cv::CAP_PROP_FPS, 30);

  if (!cap.isOpened())
  {
    cerr << "ERROR! Unable to open camera\n";
    return -1;
  }

  Mat init_frame, frame;
  cap.read(init_frame);
  while (1)
  {
    cap.read(frame);
    if (frame.empty())
    {
      cerr << "ERROR! blank frame grabbed\n";
      break;
    }

    timeval st, et;
    cuda::GpuMat cuFrameMat01, cuFrameMat02;

    gettimeofday(&st, NULL);
    cuFrameMat01.upload(init_frame);
    cuFrameMat02.upload(frame);
    //gettimeofday(&et, NULL);
    //timersub(&et, &st, &et);
    //cout << "GPU Upload: " << double(et.tv_sec) * 1000 + double(et.tv_usec) / 1000 << "ms" << endl;

    cuda::cvtColor(cuFrameMat01, cuFrameMat01, CV_BGR2GRAY);
    cuda::cvtColor(cuFrameMat02, cuFrameMat02, CV_BGR2GRAY);

    //Ptr<cuda::ORB> orb = cuda::ORB::create(1000, 1.2f, 8, 31, 0, 2, ORB::HARRIS_SCORE, 31, 20, false);
    Ptr<cuda::ORB> orb = cuda::ORB::create();

    cuda::GpuMat cuMask01(init_frame.rows, init_frame.cols, CV_8UC1, cv::Scalar::all(1)); //330,215
    cuda::GpuMat cuMask02(frame.rows, frame.cols, CV_8UC1, cv::Scalar::all(1)); //315,235

    cuda::GpuMat cuKeyPoints01, cuKeyPoints02;
    cuda::GpuMat cuDescriptors01, cuDescriptors02;

    //gettimeofday(&st, NULL);
    orb->detectAndComputeAsync(cuFrameMat01, cuMask01, cuKeyPoints01, cuDescriptors01);
    orb->detectAndComputeAsync(cuFrameMat02, cuMask02, cuKeyPoints02, cuDescriptors02);

    gettimeofday(&et, NULL);
    timersub(&et, &st, &et);
    cout << "GPU detectAndComputeAsnyc: " << double(et.tv_sec) * 1000 + double(et.tv_usec) / 1000 << "ms" << endl;

    //Ptr<cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
    //cuda::GpuMat gpuMatchesMat;
    //matcher->knnMatchAsync(cuDescriptors01, cuDescriptors02, gpuMatchesMat, 2, noArray());
    //vector<vector<DMatch>> knnMatchesVec;

    //waitKey();
  }

  waitKey(0);
  return 0;
}