#include <kernels/source/bag_camera.h>
#include <utils/local_copy_functions.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {
    BagCamera::BagCamera(std::string id): FleXRKernel(id) {
      setName("BagCamera");
      seq = 0;
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy);
    }
    BagCamera::~BagCamera() {}


    BagCamera::BagCamera(std::string id, std::string bagPath, std::string bagTopic, int targetFps): BagCamera(id)
    {
      if(!bagReader.openBag(bagPath, bagTopic)) exit(1);
      freqManager.setFrequency(targetFps);
    }


    void BagCamera::subscribeBagFile(std::string bagPath, std::string bagTopic)
    {
      if(!bagReader.openBag(bagPath, bagTopic)) exit(1);
    }


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

      debug_print("FrameInfo: %s:%d %lf", outFrame->tag, outFrame->seq, outFrame->ts);
      portManager.sendOutput<BagCameraMsgType>("out_frame", outFrame);

      freqManager.adjust();

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq, st, et, et-st);

      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

