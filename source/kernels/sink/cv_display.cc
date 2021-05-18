#include "kernels/kernel.h"
#include <kernels/sink/cv_display.h>
#include <string>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    CVDisplay::CVDisplay(): MXREKernel()
    {
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
    }

    raft::kstatus CVDisplay::run()
    {
      CVDisplayMsgType *inFrame = portManager.getInput<CVDisplayMsgType>("in_frame");

      double st = getTsNow();

      cv::imshow("CVDisplay", inFrame->data.useAsCVMat());
      int inKey = cv::waitKey(1) & 0xFF;

      double et = getTsNow();
      if(debugMode) debug_print("disp(%lf), e2e info: %s(%d:%lf)", et-st, inFrame->tag, inFrame->seq, et-inFrame->ts);
      if(logger.isSet()) logger.getInstance()->info("{} frame disp_time/e2e_wo_disp/e2e_w_disp\t{}\t{}\t{}",
                                                    inFrame->seq, et - st, st - inFrame->ts, et - inFrame->ts);

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

