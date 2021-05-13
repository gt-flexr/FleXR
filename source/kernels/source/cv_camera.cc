#include <kernels/source/cv_camera.h>
#include <utils/msg_sending_functions.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    CVCamera::CVCamera(std::string id, int dev_idx, int width, int height, int targetFps):
      MXREKernel(id), frameReader(dev_idx, width, height), freqManager(targetFps)
    {
      seq = 0;
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, 0, 0);
    }

    CVCamera::~CVCamera()
    { }

    raft::kstatus CVCamera::run()
    {
      double st = getTsNow();

      CVCameraMsgType *outFrame = portManager.getOutputPlaceholder<CVCameraMsgType>("out_frame");

      outFrame->data = frameReader.readFrame();
      strcpy(outFrame->tag, "cvcam_frame");
      outFrame->seq = seq++;
      outFrame->ts  = getTsNow();

      portManager.sendOutput<CVCameraMsgType>("out_frame", outFrame);

      freqManager.adjust();

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq-1, st, et, et-st);

      return raft::proceed;
    }

  } // namespace device
} // namespace mxre

