#include "kernels/kernel.h"
#include <kernels/sources/mock_camera.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <types/cv/types.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor */
    MockCamera::MockCamera(std::string path, int width, int height): MXREKernel(){
      this->frame_idx = 0;
      this->imagePath = path;
      this->width = width;
      this->height = height;
      addOutputPort<mxre::types::Frame>("out_frame");

      cachedFrame = cv::imread(imagePath);
      if(cachedFrame.empty()) {
        debug_print("Could not read the image: %s", imagePath.c_str());
        exit(0);
      }

      int rowPadding = height - cachedFrame.rows;
      int colPadding = width - cachedFrame.cols;
      if(rowPadding > 0 && colPadding > 0) {
        debug_print("padding : %d %d", rowPadding, colPadding);
        cv::copyMakeBorder(cachedFrame, cachedFrame, 0, rowPadding, 0, colPadding, cv::BORDER_CONSTANT,
                           cv::Scalar::all(0));
      }

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("mock_camera", "logs/" + std::to_string(pid) + "/mock_camera.log");
#endif
    }


    /* Destructor */
    MockCamera::~MockCamera() { }


    /* Kernel Run */
    raft::kstatus MockCamera::run() {
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      sleepForMS((periodMS-periodAdj >= 0) ? periodMS-periodAdj : 0); // control read frequency
      periodStart = getTimeStampNowUint();

      auto &outFrame( output["out_frame"].allocate<mxre::types::Frame>() );

      outFrame = mxre::types::Frame(cachedFrame);
      frame_idx++;
      output["out_frame"].send();
      sendFrameCopy("out_frame", &outFrame);

      periodEnd = getTimeStampNowUint();
      periodAdj = periodEnd - periodStart;

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame\t start\t{}\t end\t{}\t exe\t{}", frame_idx, startTimeStamp, endTimeStamp,
          endTimeStamp-startTimeStamp);
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

