#include <kernels/sinks/non_display.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    NonDisplay::NonDisplay()
    {
      addInputPort<mxre::types::Frame>("in_frame");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("non_display", "logs/non_display.log");
#endif
    }


    raft::kstatus NonDisplay::run()
    {
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );
#ifdef __PROFILE__
      logger->info("\t{}", getTimeStampNow());
#endif

      frame.release();
      recyclePort("in_frame");
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

