#include <kernels/sinks/non_display.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    NonDisplay::NonDisplay()
    {
      addInputPort<mxre::types::Frame>("in_frame");
    }


    raft::kstatus NonDisplay::run()
    {
#ifdef __PROFILE__
      start = getNow();
#endif
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );
      debug_print("START");

      frame.release();
      recyclePort("in_frame");

#ifdef __PROFILE__
      end = getNow();
      profile_print("Exe Time: %lf ms", getExeTime(end, start));
#endif
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

