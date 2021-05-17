#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

using namespace std;
using namespace mxre::types;
using namespace mxre::cv_types;
using namespace mxre::gl_types;

// pipeline runner for threads
void runPipeline(raft::map *pipeline) { pipeline->exe(); }

int main(int argc, char const *argv[])
{
  string mxre_home = getenv("MXRE_HOME");
  string config_yaml = mxre_home + "/examples/marker_ar/config.yaml";
  if(mxre_home.empty()) {
    cout << "Set MXRE_HOME as a environment variable" << endl;
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

  mxre::kernels::BagCamera bagCam("bag_frame", bagFile, bagTopic, bagFPS);
  bagCam.setDebugMode();
  bagCam.setLogger("bag_cam_logger", "bag_cam.log");
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<Message<Frame>>("out_frame");

  mxre::kernels::Keyboard keyboard;
  keyboard.setDebugMode();
  keyboard.activateOutPortAsRemote<Message<char>>("out_key", serverAddr, serverMessagePort);

  mxre::kernels::RTPFrameSender rtpFrameSender(serverAddr, serverFramePort, clientEncoder,
                                               width, height, width*height*4, bagFPS);
  rtpFrameSender.setDebugMode();
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<Message<Frame>>("in_frame");

  sendPipe.link(&bagCam, "out_frame", &rtpFrameSender, "in_frame", 1);
  std::thread sendThread(runPipeline, &sendPipe);
  std::thread keyThread(mxre::kernels::runSingleKernel, &keyboard);

  // 2. create & run a receiving pipeline
  raft::map recvPipe;

  mxre::kernels::RTPFrameReceiver rtpFrameReceiver(clientFramePort, clientDecoder, width, height);
  rtpFrameReceiver.setDebugMode();
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<Message<Frame>>("out_frame");

  mxre::kernels::NonDisplay nonDisplay;
  nonDisplay.setDebugMode();
  nonDisplay.setLogger("non_display_logger", "non_display.log");
  nonDisplay.activateInPortAsLocal<Message<Frame>>("in_frame");

  recvPipe += rtpFrameReceiver["out_frame"] >> nonDisplay["in_frame"];
  std::thread recevThread(runPipeline, &recvPipe);

  recevThread.join();
  keyThread.join();
  sendThread.join();

  return 0;
}

