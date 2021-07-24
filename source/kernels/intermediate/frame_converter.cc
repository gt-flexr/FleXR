#include <kernels/intermediate/frame_converter.h>
#include <utils/msg_receiving_functions.h>
#include <utils/msg_sending_functions.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    FrameConverter::FrameConverter(std::string id, int width, int height, Conversion conv): MXREKernel(id)
    {
      this->width = width; this->height = height;
      this->conv = conv;

      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::recvRemoteFrame,
                                    types::allocFrameWithBuffer);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, utils::sendRemoteFrame,
                                     types::freeFrameMsg);
    }


    raft::kstatus FrameConverter::run()
    {
      int inFrameSize, elemSize;
      switch(conv) {
      case Conversion::RGB2RGBA:
        elemSize = 3;
        inFrameSize = width*height*elemSize;
        break;
      default:
        break;
      }

      Message<Frame> *inFrame  = portManager.getInputWithSize<Message<Frame>>("in_frame", inFrameSize);
      inFrame->data.cols = width;
      inFrame->data.rows = height;
      inFrame->data.elemSize = elemSize;
      inFrame->data.totalElem = width*height;

      Message<Frame> *outFrame = portManager.getOutputPlaceholder<Message<Frame>>("out_frame");
      outFrame->ts = inFrame->ts;
      strcpy(outFrame->tag, inFrame->tag);
      outFrame->seq = inFrame->seq;

      cv::Mat temp;
      switch(conv) {
      case Conversion::RGB2RGBA:
        cv::cvtColor(inFrame->data.useAsCVMat(), temp, cv::COLOR_RGB2RGBA);
        outFrame->data = types::Frame(temp);
        break;
      default:
        break;
      }

      inFrame->data.release();
      portManager.sendOutput("out_frame", outFrame);
      portManager.freeInput("in_frame", inFrame);
      return raft::proceed;
    }
  } // namespace kernels
} // namespace mxre

