#include <flexr>

using namespace std;
using namespace flexr::kernels;

int main()
{
  string flexrHome = getenv("FLEXR_HOME");
  if(flexrHome.empty()) {
    debug_print("Set FLEXR_HOME as a environment variable"); return -1;
  }
  string configYaml = flexrHome + "/examples/tutorials/config.yaml";
  debug_print("configurations from %s", configYaml.c_str());

  YAML::Node config = YAML::LoadFile(configYaml);

  string bagFile  = config["bag_file"].as<string>();
  string bagTopic = config["bag_topic"].as<string>();
  int width       = config["width"].as<int>();
  int height      = config["height"].as<int>();
  int fps         = config["bag_fps"].as<int>();
  if(bagFile.empty() || bagTopic.empty()) {
    debug_print("Set your bag file info to %s", configYaml.c_str()); return -1;
  }

  int clientFramePort  = config["client_frame_port"].as<int>();
  string clientEncoder = config["client_encoder"].as<string>();
  string clientDecoder = config["client_decoder"].as<string>();

  string serverAddr    = config["server_addr"].as<string>();
  int serverFramePort  = config["server_frame_port"].as<int>();

  raft::map sendingPipeline;

  BagCamera bagCam("bag_cam", bagFile, bagTopic, fps);
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");

  RTPFrameSender rtpFrameSender(serverAddr, serverFramePort, clientEncoder, width, height, width*height*4, 60);
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<FrameSenderMsgType>("in_frame");

  sendingPipeline += bagCam["out_frame"] >> rtpFrameSender["in_frame"];

  std::thread sendThread(flexr::kernels::runPipeline, &sendingPipeline);

  raft::map receivingPipeline;
  RTPFrameReceiver rtpFrameReceiver(clientFramePort, clientDecoder, width, height);
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");

  NonDisplay nonDisplay;
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

  receivingPipeline += rtpFrameReceiver["out_frame"] >> nonDisplay["in_frame"];
  std::thread recvThread(flexr::kernels::runPipeline, &receivingPipeline);

  recvThread.join();
  sendThread.join();

  return 0;
}

