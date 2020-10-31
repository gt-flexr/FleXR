#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;


void runPipeline(raft::map *pipeline) { pipeline->exe(); }

class TempSrc : public raft::kernel {
  public:
  int frame_idx;
  TempSrc() : raft::kernel() {
#ifdef __PROFILE__
    frame_idx=0;
    output.addPort<mxre::types::FrameStamp>("frame_stamp");
#endif
  }

  raft::kstatus run() {
#ifdef __PROFILE__
    auto &outFrameStamp( output["frame_stamp"].allocate<mxre::types::FrameStamp>() );
    outFrameStamp.index = frame_idx++;
    outFrameStamp.st = getNow();
    output["frame_stamp"].send();
#endif

    return raft::proceed;
  }
};


class TempSink : public raft::kernel {
  public:
  TempSink() : raft::kernel() {
#ifdef __PROFILE__
    input.addPort<mxre::types::FrameStamp>("frame_stamp");
#endif
  }

  raft::kstatus run() {
#ifdef __PROFILE__
    auto &inFrameStamp( input["frame_stamp"].peek<mxre::types::FrameStamp>() );
    input["frame_stamp"].recycle();
#endif
    return raft::proceed;
  }
};


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
#ifdef __PROFILE__
  TempSink tempSink;
  sendingPipe += cam["frame_stamp"] >> tempSink["frame_stamp"];
#endif
  std::thread sendingThread(runPipeline, &sendingPipe);

  debug_print("receivingPipe");
  raft::map receivingPipe;
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", 49983, WIDTH, HEIGHT);
  mxre::kernels::CVDisplay cvDisplay;
  receivingPipe += rtpReceiver["out_data"] >> cvDisplay["in_frame"];
#ifdef __PROFILE__
  TempSrc tempSrc;
  receivingPipe += tempSrc["frame_stamp"] >> cvDisplay["frame_stamp"];
#endif

  std::thread recevingThread(runPipeline, &receivingPipe);

  sendingThread.join();
  recevingThread.join();

  return 0;
}
