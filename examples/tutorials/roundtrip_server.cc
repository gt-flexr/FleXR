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

  int width       = config["width"].as<int>();
  int height      = config["height"].as<int>();
  int fps         = config["bag_fps"].as<int>();

  string clientAddr    = config["client_addr"].as<string>();
  int clientFramePort  = config["client_frame_port"].as<int>();

  string serverAddr    = config["server_addr"].as<string>();
  int serverFramePort  = config["server_frame_port"].as<int>();
  string serverEncoder = config["server_encoder"].as<string>();
  string serverDecoder = config["server_decoder"].as<string>();

  raft::map pipeline;

  RTPFrameReceiver rtpFrameReceiver(serverFramePort, serverDecoder, width, height);
  rtpFrameReceiver.setDebugMode();
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");

  RTPFrameSender rtpFrameSender(clientAddr, clientFramePort, serverEncoder, width, height,
                                               width*height*4, fps);
  rtpFrameSender.setDebugMode();
  rtpFrameSender.setLogger("rtp_frame_sender_logger", "rtp_frame_sender.log");
  rtpFrameSender.activateInPortAsLocal<FrameSenderMsgType>("in_frame");

  pipeline += rtpFrameReceiver["out_frame"] >> rtpFrameSender["in_frame"];
  pipeline.exe();
}

