#ifndef __FLEXR_CV_CAMERA__
#define __FLEXR_CV_CAMERA__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "components/cv_frame_reader.h"
#include "components/frequency_manager.h"

namespace flexr
{
  namespace kernels
  {
    using CVCameraMsgType = types::Message<types::Frame>;

    class CVCamera : public FleXRKernel
    {
    private:
      components::CVFrameReader frameReader;
      components::FrequencyManager freqManager;
      uint32_t seq;

    public:
      CVCamera(std::string id="cv_camera", int dev_idx=0, int width=1280, int height=720, int tagetFps=30);
      ~CVCamera();
      void setIntrinsic(cv::Mat inIntrinsic);
      void setDistCoeffs(cv::Mat inDistCoeffs);
      void activateOutPortAsRemote(const std::string tag, const std::string addr, int portNumber)
      {
        debug_print("not allow remote port activation.");
      }
      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

