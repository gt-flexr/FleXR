#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

using namespace std;

class TestSink : public raft::kernel {
  public:
  TestSink(): raft::kernel() {
    input.addPort<clock_t>("in_timestamp");
  }

  virtual raft::kstatus run() {
    input["in_timestamp"].recycle();
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

  TestSink testSink;
  mxre::pipeline::input_srcs::Camera cam(camera_no);
  mxre::pipeline::network::RTPFrameSender rtpSender("mjpeg", 49990, 800000, 30, WIDTH, HEIGHT);
  raft::map pipeline;

  // cam - obj detector
  pipeline += cam["out_frame"] >> rtpSender["in_data"];
  pipeline += cam["out_timestamp"] >> testSink["in_timestamp"];

  pipeline.exe();
  return 0;
}
