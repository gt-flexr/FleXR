#include <mxre>

using namespace std;

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
  int fps         = config["bag_fps"].as<int>();

  string clientAddr    = config["client_addr"].as<string>();
  int clientFramePort  = config["client_frame_port"].as<int>();

  string serverAddr    = config["server_addr"].as<string>();
  int serverFramePort  = config["server_frame_port"].as<int>();
  string serverEncoder = config["server_encoder"].as<string>();
  string serverDecoder = config["server_decoder"].as<string>();

  raft::map pipeline;
  mxre::kernels::RTPFrameReceiver rtpFrameReceiver(serverFramePort, serverDecoder, width, height);
  mxre::kernels::RTPFrameSender rtpFrameSender(clientAddr, clientFramePort, serverEncoder, width, height,
                                               width*height*4, fps);

  pipeline += rtpFrameReceiver["out_frame"] >> rtpFrameSender["in_frame"];
  pipeline.exe();
}

