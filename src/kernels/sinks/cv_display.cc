#include <kernels/sinks/cv_display.h>
#include <string>
#include <types/cv/types.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    CVDisplay::CVDisplay()
    {
      addInputPort<mxre::types::Frame>("in_frame");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("cv_display", "logs/" + std::to_string(pid) + "/cv_display.log");
#endif
    }


    bool CVDisplay::logic(mxre::types::Frame *inFrame) {
      cv::imshow("CVDisplay", inFrame->useAsCVMat());
      int inKey = cv::waitKey(1) & 0xFF;

      inFrame->release();
      return true;
    }


    raft::kstatus CVDisplay::run()
    {
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );
      uint32_t frameIndex = frame.index;
      double frameTimestamp = frame.timestamp;

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      logic(&frame);
#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame disp_time/e2e_wo_disp/e2e_w_disp\t{}\t{}\t{}",
          frameIndex,
          endTimeStamp - startTimeStamp,
          startTimeStamp - frameTimestamp,
          endTimeStamp - frameTimestamp);
#endif
      recyclePort("in_frame");

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

