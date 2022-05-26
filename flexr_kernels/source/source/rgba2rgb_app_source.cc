#ifdef __FLEXR_KERNEL_RGBA2RGB_APP_SOURCE__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    Rgba2RgbAppSource::Rgba2RgbAppSource(std::string id, std::string shmqName, int maxShmqElem, int _width, int _height, int targetFps):
      FleXRKernel(id), width(_width), height(_height)
    {
      setName("Rgba2RgbAppSource");
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, utils::serializeRawFrame);
      freqManager.setFrequency(targetFps);

      shmQueue.initQueue(shmqName.c_str(), maxShmqElem, width*height*4);
      seq = 0;
    }


    raft::kstatus Rgba2RgbAppSource::run()
    {
      double st = getTsNow();

      if(shmQueue.isEmpty() == false)
      {
        cv::Mat rgba(height, width, CV_8UC4);
        int occupiedSize = 0;
        bool res = shmQueue.dequeueElem(rgba.data, occupiedSize, rgba.elemSize()*rgba.total());

        if(res == true)
        {
          types::Message<types::Frame> *outFrame = portManager.getOutputPlaceholder<types::Message<types::Frame>>("out_frame");

          outFrame->data = types::Frame(cv::Mat(height, width, CV_8UC3));
          outFrame->setHeader("AppFrame", seq++, getTsNow(), outFrame->data.dataSize);
          cv::cvtColor(rgba, outFrame->data.useAsCVMat(), cv::COLOR_RGBA2RGB);

          portManager.sendOutput<types::Message<types::Frame>>("out_frame", outFrame);

          double et = getTsNow();
          if(logger.isSet()) logger.getInstance()->info("{} frame\t start\t{}\t end\t{}\t exe\t{}", seq-1, st, et, et-st);
        }
      }

      freqManager.adjust();
      return raft::proceed;
    }

  }
}

#endif

