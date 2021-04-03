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

  int camIdx = config["cam_dev_idx"].as<int>();
  int width = config["width"].as<int>();
  int height = config["height"].as<int>();

  raft::map pipeline;
  mxre::kernels::CVCamera cvCam(camIdx, width, height);
  mxre::kernels::CVDisplay cvDisplay;

  pipeline += cvCam["out_frame"] >> cvDisplay["in_frame"];
  pipeline.exe();
}

