#include <kernels/source/bag_camera.h>
#include <utils/msg_sending_functions.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {
    BagCamera::BagCamera(std::string tag, std::string bagPath, std::string bagTopic, int targetFps):
      FleXRKernel(tag), freqManager(targetFps)
    {
      if(!bagReader.openBag(bagPath, bagTopic)) exit(1);
      seq = 0;
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, 0, 0);
    }

    BagCamera::~BagCamera()
    { }

    void BagCamera::setFramesToCache(int numFrames, int startFrameIndex)
    {
      bagReader.cacheFrames(numFrames, startFrameIndex);
    }

    raft::kstatus BagCamera::run()
    {
      double st = getTsNow();

      BagCameraMsgType *outFrame = portManager.getOutputPlaceholder<BagCameraMsgType>("out_frame");

      outFrame->data = bagReader.getNextFrame();
      strcpy(outFrame->tag, "bagcam_frame");
      outFrame->seq  = seq++;
      outFrame->ts   = getTsNow();

      if(debugMode) debug_print("FrameInfo: %s:%d %lf", outFrame->tag, outFrame->seq, outFrame->ts);
      portManager.sendOutput<BagCameraMsgType>("out_frame", outFrame);

      freqManager.adjust();

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq, st, et, et-st);

      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

