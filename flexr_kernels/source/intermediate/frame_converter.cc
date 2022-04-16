#ifdef __FLEXR_KERNEL_FRAME_CONVERTER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {

    FrameConverter::FrameConverter(std::string id): FleXRKernel(id)
    {
      setName("FrameConverter");
      this->width = 0; this->height = 0;
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy);
    }


    FrameConverter::FrameConverter(std::string id, int width, int height, Conversion conv): FrameConverter(id)
    {
      this->width = width; this->height = height;
      setConversion(conv);
    }


    void FrameConverter::setResolution(int width, int height)
    {
      this->width = width;
      this->height = height;

      setInFormat();
    }


    void FrameConverter::setConversion(std::string conv)
    {
      if(conv == "RGB2RGBA") this->conv = Conversion::RGB2RGBA;
      else if(conv == "RGBA2RGB") this->conv = Conversion::RGBA2RGB;
      else if(conv == "BGRA2RGB") this->conv = Conversion::BGRA2RGB;

      setInFormat();
    }


    void FrameConverter::setConversion(Conversion conv)
    {
      this->conv = conv;

      setInFormat();
    }


    void FrameConverter::setInFormat()
    {
      switch(conv) {
        case Conversion::RGB2RGBA:
          inFormat = cv::Mat(height, width, CV_8UC3);
          break;
        case Conversion::RGBA2RGB:
        case Conversion::BGRA2RGB:
          inFormat = cv::Mat(height, width, CV_8UC4);
          break;
        default:
          debug_print("Conversion type is not specified");
          break;
      }
      inFrameSize = inFormat.elemSize() * inFormat.total();
    }


    raft::kstatus FrameConverter::run()
    {
      Message<Frame> *inFrame  = portManager.getInput<Message<Frame>>("in_frame");
      inFrame->data.setFrameAttribFromCVMat(inFormat);
      inFrame->dataSize = inFrame->data.dataSize;
      if(conv == Conversion::RGBA2RGB) debug_print("received Data Size: %d/%d", inFrame->dataSize, inFrameSize);

      Message<Frame> *outFrame = portManager.getOutputPlaceholder<Message<Frame>>("out_frame");
      outFrame->ts = inFrame->ts;
      strcpy(outFrame->tag, inFrame->tag);
      outFrame->seq = inFrame->seq;

      cv::Mat temp;
      switch(conv) {
      case Conversion::RGB2RGBA:
        cv::cvtColor(inFrame->data.useAsCVMat(), temp, cv::COLOR_RGB2RGBA); break;
      case Conversion::RGBA2RGB:
        cv::cvtColor(inFrame->data.useAsCVMat(), temp, cv::COLOR_RGBA2RGB); break;
      case Conversion::BGRA2RGB:
        cv::cvtColor(inFrame->data.useAsCVMat(), temp, cv::COLOR_BGRA2RGB); break;
      default:
        break;
      }
      outFrame->data = types::Frame(temp);
      outFrame->dataSize = outFrame->data.dataSize;

      inFrame->data.release();
      portManager.sendOutput("out_frame", outFrame);
      portManager.freeInput("in_frame", inFrame);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

