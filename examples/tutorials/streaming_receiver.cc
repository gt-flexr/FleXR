#include <mxre>

using namespace std;
using namespace mxre::kernels;

int main()
{
  string mxreHome = getenv("MXRE_HOME");
  if(mxreHome.empty()) {
    debug_print("Set MXRE_HOME as a environment variable"); return -1;
  }
  string configYaml = mxreHome + "/examples/tutorials/config.yaml";
  debug_print("configurations from %s", configYaml.c_str());

  YAML::Node config = YAML::LoadFile(configYaml);

  int width       = config["width"].as<int>();
  int height      = config["height"].as<int>();

  string serverAddr    = config["server_addr"].as<string>();
  int serverFramePort  = config["server_frame_port"].as<int>();
  string serverDecoder = config["server_decoder"].as<string>();

  raft::map pipeline;
  RTPFrameReceiver rtpFrameReceiver(serverFramePort, serverDecoder, width, height);
  rtpFrameReceiver.setDebugMode();
  rtpFrameReceiver.setLogger("rtp_frame_receiver_logger", "rtp_frame_receiver.log");
  rtpFrameReceiver.activateOutPortAsLocal<FrameReceiverMsgType>("out_frame");

  CVDisplay cvDisplay;
  cvDisplay.setDebugMode();
  cvDisplay.setLogger("cv_display_logger", "cv_display.log");
  cvDisplay.activateInPortAsLocal<CVDisplayMsgType>("in_frame");

  pipeline += rtpFrameReceiver["out_frame"] >> cvDisplay["in_frame"];
  pipeline.exe();
}

