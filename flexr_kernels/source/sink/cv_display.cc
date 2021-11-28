#ifdef __FLEXR_KERNEL_CV_DISPLAY__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {

    CVDisplay::CVDisplay(std::string id): FleXRKernel(id)
    {
      setName("CVDisplay");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
    }


    raft::kstatus CVDisplay::run()
    {
      CVDisplayMsgType *inFrame = portManager.getInput<CVDisplayMsgType>("in_frame");

      double st = getTsNow();

      //debug_print("displaying frame size: %d / %d", inFrame->dataSize, inFrame->data.dataSize);

      cv::imshow(id, inFrame->data.useAsCVMat());
      int inKey = cv::waitKey(1) & 0xFF;

      double et = getTsNow();
      //debug_print("disp(%lf), e2e info: %s(%d:%d)", et-st, inFrame->tag, inFrame->seq, et-inFrame->ts);
      if(logger.isSet()) logger.getInstance()->info("{} frame disp_time/e2e_wo_disp/e2e_w_disp\t{}\t{}\t{}",
                                                    inFrame->seq, et - st, st - inFrame->ts, et - inFrame->ts);

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

