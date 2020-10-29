#include <kernels/sources/cv_camera.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {

    CVCamera::CVCamera(int dev_idx, int width, int height) : intrinsic(3, 3, CV_64FC1),
        distCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0}), width(width), height(height), raft::kernel()
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

      output.addPort<mxre::cv_types::Mat>("out_frame");
#ifdef __PROFILE__
      output.addPort<mxre::types::FrameStamp>("frame_stamp");
#endif
    }


    CVCamera::~CVCamera()
    {
      if (cam.isOpened())
        cam.release();
    }


    raft::kstatus CVCamera::run()
    {
      auto &frame( output["out_frame"].allocate<mxre::cv_types::Mat>() );
      frame = mxre::cv_types::Mat(height, width, CV_8UC3);
      cam.read(frame.cvMat);

      std::string ty =  mxre::cv_utils::type2str( frame.cvMat.type() );
      printf("Matrix: %s %dx%d \n", ty.c_str(), frame.cols, frame.rows );

      if (frame.cvMat.empty())
      {
        std::cerr << "ERROR: blank frame grabbed" << std::endl;
        exit(1);
      }

#ifdef __PROFILE__
      auto &frameStamp( output["frame_stamp"].allocate<mxre::types::FrameStamp>() );
      frameStamp.index = frame_idx;
      frameStamp.st = getNow();
      output["frame_stamp"].send();
#endif

      output["out_frame"].send();

      frame_idx++;
      if(frame_idx < TOTAL_FRAMES)
        return raft::proceed;
      else
        return raft::stop;
    }

  } // namespace device
} // namespace mxre

