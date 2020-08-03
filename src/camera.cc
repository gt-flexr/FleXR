#include <include/camera.h>
#include <include/cv_types.h>

namespace mxre
{
  namespace pipeline
  {
    namespace input_srcs
    {
      Camera::Camera(int dev_idx=0) : raft::kernel()
      {
        cam.open(dev_idx, cv::CAP_ANY);
        cam.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
        cam.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
        if (!cam.isOpened())
          std::cerr << "ERROR: unable to open camera" << std::endl;

        frame_idx = 0;

        // set default camera intrinsic
        double mat[] = {WIDTH, 0,     WIDTH / 2,
                        0,     WIDTH, HEIGHT / 2,
                        0,     0,     1};
        cv::Mat defaultIntrinsic(3, 3, CV_64FC1, mat);
        intrinsic = defaultIntrinsic;

        // set default camera distCoeffs
        double d[] = {0, 0, 0, 0};
        cv::Mat defaultDistCoeffs(4, 1, CV_64FC1, d);
        distCoeffs = defaultDistCoeffs;

        output.addPort<cv::Mat>("out_frame");
        output.addPort<clock_t>("out_timestamp");
      }

      Camera::~Camera()
      {
        if (cam.isOpened())
          cam.release();
      }

      raft::kstatus Camera::run()
      {
        while (frame_idx++ < TOTAL_FRAMES)
        {
          //auto frame(output["out_frame"].template allocate_s<cv::Mat>());
          //auto time_stamp(output["out_time_stamp"].template allocate_s<clock_t>());
          auto frame = output["out_frame"].template allocate_s<cv::Mat>();
          auto time_stamp = output["out_timestamp"].template allocate_s<clock_t>();


          cam >> *frame;
          if ((*frame).empty())
          {
            std::cerr << "ERROR: blank frame grabbed" << std::endl;
            break;
          }
          *time_stamp = clock();
          //cv::imshow("Origin Frame", *frame);
          //printf("[Camera] %d frame at st(%ld)\n", frame_idx, *time_stamp);
          output["out_frame"].send(); // zero copy
          output["out_timestamp"].send();
          return raft::proceed;
        }

        return raft::stop;
      }
    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre