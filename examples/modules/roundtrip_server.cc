#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;


int main(int argc, char const *argv[])
{
  raft::map servingPipe;

  debug_print("rtpReceiver()");
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", 49981, WIDTH, HEIGHT);

  debug_print("rtpSender()");
  mxre::kernels::RTPFrameSender rtpSender("mjpeg", "127.0.0.1", 49983, 800000, 30, WIDTH, HEIGHT);

  servingPipe += rtpReceiver["out_data"] >> rtpSender["in_data"];
  servingPipe.exe();

  return 0;
}
