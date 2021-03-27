#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1920
#define HEIGHT 1080

using namespace std;

// pipeline runner for threads
void runPipeline(raft::map *pipeline) { pipeline->exe(); }

int main(int argc, char const *argv[])
{
  // 1. create & run a sending pipeline
  raft::map sendingPipe;
  //mxre::kernels::ImageLoader imageLoader("/home/jin/github/mxre/resources/video/720p/", "video_", 1, 6, WIDTH, HEIGHT);
  mxre::kernels::ImageLoader imageLoader("/home/jin/github/mxre/resources/video/1080p/", "video_", 1, 6, WIDTH, HEIGHT);
  mxre::kernels::FFmpegRTPSender rtpSender("mjpeg", "127.0.0.1", 49985, 800000, 10, WIDTH, HEIGHT);

  imageLoader.setSleepPeriodMS(100);
  sendingPipe += imageLoader["out_frame"] >> rtpSender["in_data"];

  mxre::kernels::Keyboard keyboard;
  mxre::kernels::MessageSender<char> keySender("127.0.0.1", 49986, mxre::utils::sendPrimitive<char>);
  sendingPipe += keyboard["out_keystroke"] >> keySender["in_data"];

  std::thread sendingThread(runPipeline, &sendingPipe);

  // 2. create & run a receiving pipeline
  raft::map receivingPipe;
  mxre::kernels::FFmpegRTPReceiver rtpReceiver("mjpeg", "127.0.0.1", 49987, WIDTH, HEIGHT);
  //mxre::kernels::CVDisplay cvDisplay;
  mxre::kernels::NonDisplay nonDisplay;
  receivingPipe += rtpReceiver["out_data"] >> nonDisplay["in_frame"];
  std::thread recevingThread(runPipeline, &receivingPipe);

  sendingThread.join();
  recevingThread.join();

  return 0;
}

