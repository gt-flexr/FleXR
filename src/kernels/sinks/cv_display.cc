#include <kernels/sinks/cv_display.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    CVDisplay::CVDisplay()
    {
      addInputPort<mxre::types::Frame>("in_frame");
#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("cv_display", "logs/cv_display.log");
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
      startTimeStamp = getTimeStampNow();
#endif

      debug_print("START");

      logic(&frame);

      recyclePort("in_frame");
      debug_print("END");

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

