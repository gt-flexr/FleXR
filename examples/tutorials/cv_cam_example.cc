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

  int camIdx = config["cam_dev_idx"].as<int>();
  int width = config["width"].as<int>();
  int height = config["height"].as<int>();

  raft::map pipeline;
  CVCamera cvCam("cvcam", camIdx, width, height);
  cvCam.setLogger("cv_cam_logger", "cv_cam.log");
  cvCam.activateOutPortAsLocal<CVCameraMsgType>("out_frame");

  CVDisplay cvDisplay;
  cvDisplay.activateInPortAsLocal<NonDisplayMsgType>("in_frame");
  cvDisplay.setLogger("cv_display_logger", "cv_display.log");

  pipeline += cvCam["out_frame"] >> cvDisplay["in_frame"];
  pipeline.exe();
}

