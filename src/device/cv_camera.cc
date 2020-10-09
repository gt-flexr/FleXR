#include "device/cv_camera.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace device
    {
      std::string type2str(int type) {
        std::string r;

        uchar depth = type & CV_MAT_DEPTH_MASK;
        uchar chans = 1 + (type >> CV_CN_SHIFT);

        switch ( depth ) {
          case CV_8U:  r = "8U"; break;
          case CV_8S:  r = "8S"; break;
          case CV_16U: r = "16U"; break;
          case CV_16S: r = "16S"; break;
          case CV_32S: r = "32S"; break;
          case CV_32F: r = "32F"; break;
          case CV_64F: r = "64F"; break;
          default:     r = "User"; break;
        }

        r += "C";
        r += (chans+'0');

        return r;
      }


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

        output.addPort<mxre::cv_units::Mat>("out_frame");
#ifdef __PROFILE__
        output.addPort<FrameStamp>("frame_stamp");
#endif
      }

      CVCamera::~CVCamera()
      {
        if (cam.isOpened())
          cam.release();
      }

      raft::kstatus CVCamera::run()
      {
        auto &frame( output["out_frame"].allocate<mxre::cv_units::Mat>() );
        frame = mxre::cv_units::Mat(height, width, CV_8UC3);
        cam.read(frame.cvMat);

        std::string ty =  type2str( frame.cvMat.type() );
        printf("Matrix: %s %dx%d \n", ty.c_str(), frame.cols, frame.rows );

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
    } // namespace device
  }   // namespace pipeline
} // namespace mxre

