#include <kernels/sink/non_display.h>
#include <string>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    NonDisplay::NonDisplay()
    {
      addInputPort<types::Message<types::Frame>>("in_frame");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("non_display", "logs/" + std::to_string(pid) + "/non_display.log");
#endif
    }


    raft::kstatus NonDisplay::run()
    {
      types::Message<types::Frame> frame = input["in_frame"].peek<types::Message<types::Frame>>();

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame\t E2E latency\t{}", frame.seq, endTimeStamp - frame.ts);
#endif

      frame.data.release();
      recyclePort("in_frame");
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

