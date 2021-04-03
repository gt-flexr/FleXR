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

  string imageDir = config["img_dir"].as<string>();
  string fileStem = config["img_file_stem"].as<string>();
  int startIdx = config["img_start_idx"].as<int>();
  int maxPlaceValue = config["img_max_place_value"].as<int>();
  if(imageDir.empty() || fileStem.empty()) {
    debug_print("Put your image file info to %s", configYaml.c_str()); return -1;
  }

  int width = config["width"].as<int>();
  int height = config["height"].as<int>();

  raft::map pipeline;
  mxre::kernels::ImageLoader imageLoader(imageDir + "/", fileStem, startIdx, maxPlaceValue, width, height);
  mxre::kernels::CVDisplay cvDisplay;

  pipeline += imageLoader["out_frame"] >> cvDisplay["in_frame"];
  pipeline.exe();
}

