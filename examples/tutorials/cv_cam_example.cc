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

  int camIdx = config["cam_dev_idx"].as<int>();
  int width = config["width"].as<int>();
  int height = config["height"].as<int>();

  raft::map pipeline;
  CVCamera cvCam("cvcam", camIdx, width, height);
  cvCam.activateOutPortAsLocal<CVCameraMsgType>("out_frame");

  CVDisplay cvDisplay;
  cvDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");

  pipeline += cvCam["out_frame"] >> cvDisplay["in_frame"];
  pipeline.exe();
}

