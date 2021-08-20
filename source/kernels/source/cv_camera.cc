#include <kernels/source/cv_camera.h>
#include <utils/msg_sending_functions.h>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {
    CVCamera::CVCamera(std::string id, int devIdx, int width, int height, int targetFps):
      FleXRKernel(id), frameReader(devIdx, width, height)
    {
      setName("CVCamera");
      seq = 0;
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, 0, 0);
      freqManager.setFrequency(targetFps);
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
      debug_print("FrameInfo: %s:%d %lf", outFrame->tag, outFrame->seq, outFrame->ts);

      portManager.sendOutput<CVCameraMsgType>("out_frame", outFrame);

      freqManager.adjust();

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq-1, st, et, et-st);

      return raft::proceed;
    }

  } // namespace device
} // namespace flexr

