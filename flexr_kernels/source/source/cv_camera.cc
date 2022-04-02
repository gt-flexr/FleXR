#ifdef __FLEXR_KERNEL_CV_CAMERA__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    CVCamera::CVCamera(std::string id, int devIdx, int width, int height, int targetFps):
      FleXRKernel(id), frameReader(devIdx, width, height)
    {
      setName("CVCamera");
      seq = 0;
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy);
      freqManager.setFrequency(targetFps);
    }


    CVCamera::CVCamera(std::string id, std::string fileName, int width, int height, int targetFps):
      FleXRKernel(id), frameReader(fileName, width, height)
    {
      setName("CVCamera");
      seq = 0;
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy);
      freqManager.setFrequency(targetFps);
    }


    CVCamera::~CVCamera()
    { }


    raft::kstatus CVCamera::run()
    {
      //double st = getTsNow();

      CVCameraMsgType *outFrame = portManager.getOutputPlaceholder<CVCameraMsgType>("out_frame");

      outFrame->data = frameReader.readFrame();
      outFrame->setHeader("cvcam_frame", seq++, getTsNow(), outFrame->data.useAsCVMat().total()*outFrame->data.useAsCVMat().elemSize());

      portManager.sendOutput<CVCameraMsgType>("out_frame", outFrame);

      //double et = getTsNow();
      //if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq-1, st, et, et-st);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace device
} // namespace flexr

#endif

