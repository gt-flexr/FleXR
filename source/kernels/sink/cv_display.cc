#include <kernels/sink/cv_display.h>
#include <string>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    CVDisplay::CVDisplay()
    {
      addInputPort<types::Message<types::Frame>>("in_frame");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("cv_display", "logs/" + std::to_string(pid) + "/cv_display.log");
#endif
    }


    bool CVDisplay::logic(mxre::types::Frame *inFrame) {
      return true;
    }


    raft::kstatus CVDisplay::run()
    {
      types::Message<types::Frame> frame = input["in_frame"].peek<types::Message<types::Frame>>();

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      cv::imshow("CVDisplay", frame.data.useAsCVMat());
      int inKey = cv::waitKey(1) & 0xFF;

      frame.data.release();

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame disp_time/e2e_wo_disp/e2e_w_disp\t{}\t{}\t{}",
          frame.seq,
          endTimeStamp - startTimeStamp,
          startTimeStamp - frame.ts,
          endTimeStamp - frame.ts);
#endif
      recyclePort("in_frame");

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

