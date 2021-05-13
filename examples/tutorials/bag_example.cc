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

  string bagFile = config["bag_file"].as<string>();
  string bagTopic = config["bag_topic"].as<string>();
  int fps = config["bag_fps"].as<int>();
  if(bagFile.empty() || bagTopic.empty()) {
    debug_print("Set your bag file info to %s", configYaml.c_str()); return -1;
  }

  raft::map pipeline;
  BagCamera bagCam("bag_cam", bagFile, bagTopic, fps);
  bagCam.setFramesToCache(400, 400);
  bagCam.activateOutPortAsLocal<BagCameraMsgType>("out_frame");

  CVDisplay cvDisplay;
  cvDisplay.activateInPortAsLocal<CVDisplayMsgType>("in_frame");

  pipeline += bagCam["out_frame"] >> cvDisplay["in_frame"];
  pipeline.exe();
}

