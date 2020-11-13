#include <kernels/sinks/cv_display.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    CVDisplay::CVDisplay()
    {
      addInputPort<mxre::types::Frame>("in_frame");
    }


    bool CVDisplay::logic(mxre::types::Frame *inFrame) {
      cv::imshow("CVDisplay", inFrame->useAsCVMat());
      int inKey = cv::waitKey(2) & 0xFF;

      inFrame->release();
      return true;
    }


    raft::kstatus CVDisplay::run()
    {
#ifdef __PROFILE__
      start = getNow();
#endif
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );
      debug_print("START");

      logic(&frame);

      recyclePort("in_frame");
      debug_print("END");

#ifdef __PROFILE__
      end = getNow();
      profile_print("Exe Time: %lf ms", getExeTime(end, start));
#endif
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

