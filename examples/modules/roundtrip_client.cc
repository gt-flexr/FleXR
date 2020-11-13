#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;


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

  debug_print("sendingPipe");
  raft::map sendingPipe;
  mxre::kernels::CVCamera cam(camera_no);
  mxre::kernels::RTPFrameSender rtpSender("mjpeg", "127.0.0.1", 49981, 800000, 30, WIDTH, HEIGHT);
  sendingPipe += cam["out_frame"] >> rtpSender["in_data"];
  std::thread sendingThread(runPipeline, &sendingPipe);

  debug_print("receivingPipe");
  raft::map receivingPipe;
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", 49983, WIDTH, HEIGHT);
  mxre::kernels::CVDisplay cvDisplay;
  receivingPipe += rtpReceiver["out_data"] >> cvDisplay["in_frame"];

  std::thread recevingThread(runPipeline, &receivingPipe);

  sendingThread.join();
  recevingThread.join();

  return 0;
}
