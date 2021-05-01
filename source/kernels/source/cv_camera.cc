#include <kernels/source/cv_camera.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor */
    CVCamera::CVCamera(int dev_idx, int width, int height) : MXREKernel(), intrinsic(3, 3, CV_64FC1),
        distCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0}), width(width), height(height)
    {
      cam.open(dev_idx, cv::CAP_ANY);
      cam.set(cv::CAP_PROP_FRAME_WIDTH, width);
      cam.set(cv::CAP_PROP_FRAME_HEIGHT, height);
      if (!cam.isOpened())
        std::cerr << "ERROR: unable to open camera" << std::endl;

      frameIndex = 0;

      // set default camera intrinsic
      this->intrinsic.at<double>(0, 0) = width;
      this->intrinsic.at<double>(0, 1) = 0;
      this->intrinsic.at<double>(0, 2) = width/2;

      this->intrinsic.at<double>(1, 0) = 0;
      this->intrinsic.at<double>(1, 1) = width;
      this->intrinsic.at<double>(1, 2) = height/2;

      this->intrinsic.at<double>(2, 0) = 0;
      this->intrinsic.at<double>(2, 1) = 0;
      this->intrinsic.at<double>(2, 2) = 1;

      addOutputPort<mxre::types::Frame>("out_frame");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("cv_camera", "logs/" + std::to_string(pid) + "/cv_camera.log");
#endif
    }


    /* Destructor */
    CVCamera::~CVCamera() {
      if (cam.isOpened()) cam.release();
    }


    /* Kernel Logic */
    bool CVCamera::logic(mxre::types::Frame *outFrame) {
      *outFrame = mxre::types::Frame(height, width, CV_8UC3, 0, 0);
      cv::Mat outFrameAsCVMat = outFrame->useAsCVMat();
      cam.read(outFrameAsCVMat);
      outFrame->index = frameIndex++;
      outFrame->timestamp = getTimeStampNow();

      if(outFrameAsCVMat.empty()) {
        std::cerr << "ERROR: blank frame grabbed" << std::endl;
        return false;
      }
      return true;
    }


    /* Kernel Run */
    raft::kstatus CVCamera::run()
    {
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      sleepForMS((periodMS-periodAdj >= 0) ? periodMS-periodAdj : 0); // control read frequency
      periodStart = getTimeStampNowUint();

      auto &outFrame(output["out_frame"].allocate<mxre::types::Frame>());

      if(logic(&outFrame)) {
        sendFrames("out_frame", &outFrame);
      }

      periodEnd = getTimeStampNowUint();
      periodAdj = periodEnd - periodStart;


#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame\t start\t{}\t end\t{}\t exe\t{}", frameIndex-1, startTimeStamp, endTimeStamp,
                    endTimeStamp-startTimeStamp);
#endif
      return raft::proceed;
    }

  } // namespace device
} // namespace mxre

