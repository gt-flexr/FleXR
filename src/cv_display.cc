#include "cv_display.h"
#include "cv_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace output_sinks
    {
      CVDisplay::CVDisplay()
      {
        input.addPort<mxre::cv_units::Mat>("in_frame");

#ifdef __PROFILE__
        input.addPort<FrameStamp>("frame_stamp");
#endif
      }

      CVDisplay::~CVDisplay() {}

      raft::kstatus CVDisplay::run()
      {

#ifdef __PROFILE__
        TimeVal start = getNow();
#endif

        auto &frame( input["in_frame"].peek<mxre::cv_units::Mat>() );

        cv::imshow("CVDisplay", frame.cvMat);
        int inKey = cv::waitKey(10) & 0xFF;
        frame.release();

        input["in_frame"].recycle();

#ifdef __PROFILE__
        TimeVal end = getNow();
        debug_print("Exe Time: %lfms", getExeTime(end, start));

        auto &inFrameStamp( input["frame_stamp"].peek<FrameStamp>() );
        debug_print("Frame(%d) Processing Time %lfms", inFrameStamp.index, getExeTime(end, inFrameStamp.st));
        input["frame_stamp"].recycle();
#endif

        return raft::proceed;
      }
    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre

