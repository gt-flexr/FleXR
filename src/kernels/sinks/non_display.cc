#include <kernels/sinks/non_display.h>
#include <string>
#include <types/cv/types.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    NonDisplay::NonDisplay()
    {
      addInputPort<mxre::types::Frame>("in_frame");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("non_display", "logs/" + std::to_string(pid) + "/non_display.log");
#endif
    }


    raft::kstatus NonDisplay::run()
    {
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame\t E2E latency\t{}", frame.index, endTimeStamp - frame.timestamp);
#endif

      frame.release();
      recyclePort("in_frame");
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

