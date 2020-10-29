#include <kernels/sinks/cv_display.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    CVDisplay::CVDisplay()
    {
      input.addPort<mxre::cv_types::Mat>("in_frame");

#ifdef __PROFILE__
      input.addPort<mxre::types::FrameStamp>("frame_stamp");
#endif
    }

    CVDisplay::~CVDisplay() {}

    raft::kstatus CVDisplay::run()
    {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      auto &frame( input["in_frame"].peek<mxre::cv_types::Mat>() );

      cv::imshow("CVDisplay", frame.cvMat);
      int inKey = cv::waitKey(10) & 0xFF;
      frame.release();

      input["in_frame"].recycle();

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));

      auto &inFrameStamp( input["frame_stamp"].peek<mxre::types::FrameStamp>() );
      profile_print("Frame(%d) Processing Time %lfms", inFrameStamp.index, getExeTime(end, inFrameStamp.st));
      input["frame_stamp"].recycle();
#endif

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre
