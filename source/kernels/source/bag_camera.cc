#include <kernels/source/bag_camera.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    BagCamera::BagCamera(std::string tag, std::string bagPath, std::string bagTopic): MXREKernel(tag)
    {
      if(!bagReader.openBag(bagPath, bagTopic)) exit(1);
      frameIndex = 0;
      addOutputPort<types::Message<types::Frame>>("out_frame");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("bag_camera", "logs/" + std::to_string(pid) + "/bag_camera.log");
#endif
    }


    BagCamera::~BagCamera() { }


    void BagCamera::setFramesToCache(int numFrames, int startFrameIndex)
    {
      bagReader.cacheFrames(numFrames, startFrameIndex);
    }


    raft::kstatus BagCamera::run()
    {
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif
      sleepForMS((periodMS-periodAdj >= 0) ? periodMS-periodAdj : 0); // control read frequency
      periodStart = getTimeStampNowUint();

      types::Message<types::Frame> &outFrame = output["out_frame"].allocate<types::Message<types::Frame>>();

      outFrame.data = bagReader.getNextFrame();
      outFrame.tag  = tag;
      outFrame.seq  = frameIndex++;
      outFrame.ts   = getTimeStampNow();

      sendFrames("out_frame", outFrame);

      periodEnd = getTimeStampNowUint();
      periodAdj = periodEnd - periodStart;

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}th frame\t start\t{}\t end\t{}\t exe\t{}", frameIndex-1, startTimeStamp, endTimeStamp,
          endTimeStamp-startTimeStamp);
#endif
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

