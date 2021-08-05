#include "kernels/kernel.h"
#include <kernels/sink/cv_display.h>
#include <string>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {

    CVDisplay::CVDisplay(int width, int height, FrameType frameType): FleXRKernel()
    {
      this->width = width, this->height = height;
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
    }

    raft::kstatus CVDisplay::run()
    {
      CVDisplayMsgType *inFrame = portManager.getInput<CVDisplayMsgType>("in_frame");

      double st = getTsNow();

      debug_print("displaying frame size: %d / %d", inFrame->dataSize, inFrame->data.dataSize);

      //cv::imshow("CVDisplay", inFrame->data.useAsCVMat());
      //int inKey = cv::waitKey(1) & 0xFF;

      double et = getTsNow();
      if(debugMode) debug_print("disp(%lf), e2e info: %s(%d:%lf)", et-st, inFrame->tag, inFrame->seq, et-inFrame->ts);
      if(logger.isSet()) logger.getInstance()->info("{} frame disp_time/e2e_wo_disp/e2e_w_disp\t{}\t{}\t{}",
                                                    inFrame->seq, et - st, st - inFrame->ts, et - inFrame->ts);

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

