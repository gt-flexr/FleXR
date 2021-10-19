
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <raft>
#include <bits/stdc++.h>
#include <cv.hpp>
#include "kernels/perceptions/kimera/kimera_vio.h"
#include "types/kimera/types.h"
#include "kernels/sinks/illixr_appsink.h"
#include "kernels/sources/illixr_appsource.h"
#include "kernels/sinks/illixr_appzmqsink.h"
#include "kernels/sources/illixr_appzmqsource.h"

#include <kimera-vio/frontend/Camera.h>

using namespace std;

#define WIDTH 752
#define HEIGHT 480

int main(int argc, char const *argv[])
{
  
  if (argc < 3){
    std::cerr << "Usage: ./illixr_local_server <this machine ip address> <ILLIXR machine ip address>" << std::endl;
    return 1;
  }
  raft::map xrPipeline;
  mxre::kernels::IllixrAppZMQSource<mxre::kimera_type::imu_cam_type> appsource{};
  mxre::kernels::IllixrAppZMQSink<mxre::kimera_type::kimera_output> appsink{};
  mxre::kernels::KimeraVIOKernel KimeraVIOKernel;

  appsource.setup(argv[1]);
  appsink.setup(argv[2]);

  // TODO: message is not found
  // xrPipeline += ffmpeg["imu_data"] >>  KimeraVIOKernel["illixr_cam_input"];
  // xrPipeline += message["cam_data"] >>  KimeraVIOKernel["illixr_imu_input"];

  xrPipeline += appsource["out_data"] >> KimeraVIOKernel["illixr_imu_cam_input"];
  xrPipeline += KimeraVIOKernel["kimera_pose"] >> appsink["in_data"];

  xrPipeline.exe();

  return 0;
}

