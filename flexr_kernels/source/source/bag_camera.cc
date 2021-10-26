#ifdef __FLEXR_KERNEL_BAG_CAMERA__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

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
      outFrame->setHeader("bag_frame", seq++, getTsNow(), outFrame->data.useAsCVMat().total()*outFrame->data.useAsCVMat().elemSize());
      outFrame->printHeader();

      portManager.sendOutput<BagCameraMsgType>("out_frame", outFrame);


      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq, st, et, et-st);

      freqManager.adjust();
      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

#endif

