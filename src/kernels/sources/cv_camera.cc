#include <kernels/sources/cv_camera.h>
#include <types/cv/types.h>

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

      frame_idx = 0;

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
    }


    /* Destructor */
    CVCamera::~CVCamera() {
      if (cam.isOpened()) cam.release();
    }


    /* Kernel Logic */
    bool CVCamera::logic(mxre::types::Frame *outFrame) {
      *outFrame = mxre::types::Frame(height, width, CV_8UC3);
      cv::Mat outFrameAsCVMat = outFrame->useAsCVMat();
      cam.read(outFrameAsCVMat);
      debug_print("%p %p", outFrame->data, outFrameAsCVMat.data);
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
      start = getNow();
#endif
      debug_print("START");
      auto &outFrame(output["out_frame"].allocate<mxre::types::Frame>());

      if(logic(&outFrame)) {
        output["out_frame"].send();
        sendFrameCopy("out_frame", &outFrame);
      }

      debug_print("END");
#ifdef __PROFILE__
      end = getNow();
      profile_print("Exe Time: %lf ms", getExeTime(end, start));
#endif
      return raft::proceed;
    }

  } // namespace device
} // namespace mxre

