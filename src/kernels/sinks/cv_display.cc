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
      frameIndex = 0;
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("cv_display", "logs/" + std::to_string(pid) + "/cv_display.log");
#endif
    }


    bool CVDisplay::logic(mxre::types::Frame *inFrame) {
      cv::imshow("CVDisplay", inFrame->useAsCVMat());
      int inKey = cv::waitKey(2) & 0xFF;

      inFrame->release();
      return true;
    }


    raft::kstatus CVDisplay::run()
    {
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame\t Processed Time\t{}", frameIndex, endTimeStamp);
#endif
      frameIndex++;
      logic(&frame);
      recyclePort("in_frame");

      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

