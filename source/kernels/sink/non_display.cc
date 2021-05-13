#include <kernels/sink/non_display.h>
#include <string>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    NonDisplay::NonDisplay()
    {
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
    }


    raft::kstatus NonDisplay::run()
    {
      NonDisplayMsgType *inFrame = portManager.getInput<NonDisplayMsgType>("in_frame");

      double et = getTsNow();

      if(logger.isSet()) logger.getInstance()->info("{}frame\t E2E latency\t{}", inFrame->seq, et - inFrame->ts);

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

