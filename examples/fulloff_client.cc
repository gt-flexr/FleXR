#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;

// pipeline runner for threads
void runPipeline(raft::map *pipeline) { pipeline->exe(); }

int main(int argc, char const *argv[])
{
  cv::CommandLineParser parser(argc, argv,
      "{@input_path |0|input path can be a camera id, like 0,1,2 or a video filename}");
  parser.printMessage();
  string input_path = parser.get<string>(0);
  string video_name = input_path;
  int camera_no;
  camera_no = input_path[0] - '0';

  // 1. create & run a sending pipeline
  raft::map sendingPipe;
  mxre::kernels::CVCamera cam(camera_no);
  mxre::kernels::RTPFrameSender rtpSender("mjpeg", "127.0.0.1", 49985, 800000, 10, WIDTH, HEIGHT);
  sendingPipe += cam["out_frame"] >> rtpSender["in_data"];

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::StaticSender<char> keySender("127.0.0.1", 49986, false);
  sendingPipe += keyboard["out_keystroke"] >> keySender["in_data"];

  std::thread sendingThread(runPipeline, &sendingPipe);

  // 2. create & run a receiving pipeline
  raft::map receivingPipe;
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", 49987, WIDTH, HEIGHT);
  mxre::kernels::CVDisplay cvDisplay;
  receivingPipe += rtpReceiver["out_data"] >> cvDisplay["in_frame"];
  std::thread recevingThread(runPipeline, &receivingPipe);

  sendingThread.join();
  recevingThread.join();

  return 0;
}

