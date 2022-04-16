#ifdef __FLEXR_KERNEL_RGB2RGBA_APP_SINK__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    Rgb2RgbaAppSink::Rgb2RgbaAppSink(std::string id, std::string shmqName, int maxShmqElem, int _width, int _height, int targetFps):
      FleXRKernel(id), width(_width), height(_height)
    {
      setName("Rgb2RgbaAppSink");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      freqManager.setFrequency(targetFps);

      shmQueue.initQueue(shmqName.c_str(), maxShmqElem, width*height*4);
    }


    raft::kstatus Rgb2RgbaAppSink::run()
    {
      types::Message<types::Frame> *inFrame = portManager.getInput<types::Message<types::Frame>>("in_frame");
      double st = getTsNow();

      if(shmQueue.isFull() == false)
      {
        cv::Mat rgba;
        cv::cvtColor(inFrame->data.useAsCVMat(), rgba, cv::COLOR_RGB2RGBA);
        bool res = shmQueue.enqueueElem(rgba.data, rgba.elemSize()*rgba.total());
        if(res)
        {
          double et = getTsNow();
          if(logger.isSet()) logger.getInstance()->info("{}frame\t E2E latency\t{}", inFrame->seq, et - inFrame->ts);
        }
      }

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      freqManager.adjust();
      return raft::proceed;

    }
  } // namespace kernels
} // namespace flexr

#endif

