#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

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
  mxre::pipeline::input_srcs::Camera cam(camera_no);
  mxre::pipeline::network::RTPFrameSender rtpSender("mjpeg", "send.sdp", 49991, 800000, 10, WIDTH, HEIGHT);
  sendingPipe += cam["out_frame"] >> rtpSender["in_data"];
  std::thread sendingThread(runPipeline, &sendingPipe);

  // 2. create & run a receiving pipeline
  raft::map receivingPipe;
  mxre::pipeline::network::RTPFrameReceiver rtpReceiver("mjpeg", "recv.sdp", WIDTH, HEIGHT);
  mxre::pipeline::output_sinks::CVDisplay cvDisplay;
  receivingPipe += rtpReceiver["out_data"] >> cvDisplay["in_frame"];
  std::thread recevingThread(runPipeline, &receivingPipe);

  sendingThread.join();
  recevingThread.join();

  return 0;
}

