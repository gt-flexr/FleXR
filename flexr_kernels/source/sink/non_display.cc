#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    NonDisplay::NonDisplay(std::string id): FleXRKernel(id)
    {
      setName("NonDisplay");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
    }


    raft::kstatus NonDisplay::run()
    {
      NonDisplayMsgType *inFrame = portManager.getInput<NonDisplayMsgType>("in_frame");

      double et = getTsNow();

      debug_print("e2e info: %s(%d:%lf)", inFrame->tag, inFrame->seq, et-inFrame->ts);
      if(logger.isSet()) logger.getInstance()->info("{}frame\t E2E latency\t{}", inFrame->seq, et - inFrame->ts);

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);
      return raft::proceed;

    }
  } // namespace kernels
} // namespace flexr

