#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>


using namespace std;


int main(int argc, char const *argv[])
{
  raft::map servingPipe;

  debug_print("rtpReceiver()");
  mxre::pipeline::network::RTPFrameReceiver rtpReceiver("mjpeg", "send.sdp", WIDTH, HEIGHT);

  debug_print("rtpSender()");
  mxre::pipeline::network::RTPFrameSender rtpSender("mjpeg", "recv.sdp", 49991, 800000, 30, WIDTH, HEIGHT);

  servingPipe += rtpReceiver["out_data"] >> rtpSender["in_data"];
  servingPipe.exe();

  return 0;
}

