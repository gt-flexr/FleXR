#include "camera.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace input_srcs
    {
      Camera::Camera(int dev_idx) : intrinsic(3, 3, CV_64FC1), distCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0}), raft::kernel()
      {
        cam.open(dev_idx, cv::CAP_ANY);
        cam.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
        cam.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
        if (!cam.isOpened())
          std::cerr << "ERROR: unable to open camera" << std::endl;

        frame_idx = 0;

        // set default camera intrinsic
        this->intrinsic.at<double>(0, 0) = WIDTH; this->intrinsic.at<double>(0, 1) = 0; this->intrinsic.at<double>(0, 2) = WIDTH/2;
        this->intrinsic.at<double>(1, 0) = 0; this->intrinsic.at<double>(1, 1) = WIDTH; this->intrinsic.at<double>(1, 2) = HEIGHT/2;
        this->intrinsic.at<double>(2, 0) = 0; this->intrinsic.at<double>(2, 1) = 0; this->intrinsic.at<double>(2, 2) = 1;

        output.addPort<mxre::cv_units::Mat>("out_frame");
#ifdef __PROFILE__
        output.addPort<FrameStamp>("frame_stamp");
#endif
      }

      Camera::~Camera()
      {
        if (cam.isOpened())
          cam.release();
      }

      raft::kstatus Camera::run()
      {
        auto &frame( output["out_frame"].allocate<mxre::cv_units::Mat>() );
        frame = mxre::cv_units::Mat(HEIGHT, WIDTH, CV_8UC3);
        cam.read(frame.cvMat);

        if (frame.cvMat.empty())
        {
          std::cerr << "ERROR: blank frame grabbed" << std::endl;
          exit(1);
        }

#ifdef __PROFILE__
        auto &frameStamp( output["frame_stamp"].allocate<FrameStamp>() );
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
    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre

