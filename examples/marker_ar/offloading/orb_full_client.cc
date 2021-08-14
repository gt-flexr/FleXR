#include <raft>
#include <flexr>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace flexr::types;
using namespace flexr::cv_types;
using namespace flexr::gl_types;

// pipeline runner for threads
void runPipeline(raft::map *pipeline) { pipeline->exe(); }

int main(int argc, char const *argv[])
{
  string flexr_home = getenv("FLEXR_HOME");
  string config_yaml = flexr_home + "/examples/marker_ar/config.yaml";
  if(flexr_home.empty()) {
    cout << "Set FLEXR_HOME as a environment variable" << endl;
    return 0;
  }
  else cout << config_yaml << endl;

  YAML::Node config = YAML::LoadFile(config_yaml);
  int width            = config["width"].as<int>();
  int height           = config["height"].as<int>();
  string bagFile       = config["bag_file"].as<string>();
  string bagTopic      = config["bag_topic"].as<string>();
  int bagFPS           = config["bag_fps"].as<int>();
  if(bagFile.empty() || bagTopic.empty() || bagFPS == 0) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  int clientFramePort  = config["client_frame_port"].as<int>();
  string clientEncoder = config["client_encoder"].as<string>();
  string clientDecoder = config["client_decoder"].as<string>();

  string serverAddr     = config["server_addr"].as<string>();
  int serverFramePort   = config["server_frame_port"].as<int>();
  int serverMessagePort = config["server_message_port"].as<int>();

  if(clientEncoder.empty() || clientDecoder.empty() || serverAddr.empty()) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }

  // 1. create & run a sending pipeline
  raft::map sendPipe;

  flexr::kernels::BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<Message<Frame>>("out_frame");

  flexr::kernels::Keyboard keyboard;
  keyboard.activateOutPortAsRemote<Message<char>>("out_key", serverAddr, serverMessagePort);

  flexr::kernels::RTPFrameSender rtpFrameSender("rtp_frame_sender", serverAddr, serverFramePort, clientEncoder,
                                                width, height, width*height*4, bagFPS);
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<Message<Frame>>("in_frame");

  sendPipe.link(&bagCam, "out_frame", &rtpFrameSender, "in_frame", 1);
  std::thread sendThread(runPipeline, &sendPipe);
  std::thread keyThread(flexr::kernels::runSingleKernel, &keyboard);

  // 2. create & run a receiving pipeline
  raft::map recvPipe;

  flexr::kernels::RTPFrameReceiver rtpFrameReceiver("rtp_frame_receiver", clientFramePort, clientDecoder, width, height);
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<Message<Frame>>("out_frame");

  flexr::kernels::NonDisplay nonDisplay("non_display");
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<Message<Frame>>("in_frame");

  recvPipe += rtpFrameReceiver["out_frame"] >> nonDisplay["in_frame"];
  std::thread recevThread(runPipeline, &recvPipe);

  recevThread.join();
  keyThread.join();
  sendThread.join();

  return 0;
}

