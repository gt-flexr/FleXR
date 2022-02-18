#ifdef __FLEXR_KERNEL_NVMPI_ENCODER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    /* Constructor() */
    NvmpiEncoder::NvmpiEncoder(std::string id, std::string encoderName, int width, int height, int bitrate, int fps, int qp):
      FleXRKernel(id), encoderName(encoderName), width(width), height(height)
    {
      setName("NvmpiEncoder");
      portManager.registerInPortTag ("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerOutPortTag("out_encoded_frame", utils::sendLocalPointerMsgCopy, utils::serializeEncodedFrame);

      nvCodingType enc;
      if(encoderName == "h264_nvmpi") enc = NV_VIDEO_CodingH264;
      else if(encoderName == "hevc_nvmpi") enc = NV_VIDEO_CodingHEVC;
      encoder = nvpInitEncoder(enc, width, height, bitrate, 0, 24, 12, fps, 77, 50, 1, 10, qp, qp+5, 0, 0, 0);
      yuvFrame = cv::Mat::zeros(height*1.5, width, CV_8UC1);
    }


    /* Destructor() */
    NvmpiEncoder::~NvmpiEncoder()
    {
      nvpCloseEncoder(encoder);
    }


    /* Run() */
    raft::kstatus NvmpiEncoder::run()
    {
      NvEncoderOutEncodedFrameType *outEncodedFrame = portManager.getOutputPlaceholder<NvEncoderOutEncodedFrameType>("out_encoded_frame");

      NvEncoderInFrameType *inFrame = portManager.getInput<NvEncoderInFrameType>("in_frame");
      if(inFrame->data.rows != (size_t)height || inFrame->data.cols != (size_t)width) {
        debug_print("inMat size is not compatible.");
        inFrame->data.release();
        portManager.freeInput("in_frame", inFrame);
        return raft::proceed;
      }
      inFrame->printHeader();

      double st = getTsNow();
      cv::cvtColor(inFrame->data.useAsCVMat(), yuvFrame, cv::COLOR_RGB2YUV_YV12);
      nvPacket pkt = nvpEncodeYuvFrame(encoder, yuvFrame, width, height);
      double enct = getTsNow();

      outEncodedFrame->data = new uint8_t[pkt.payload_size];
      memcpy(outEncodedFrame->data, pkt.payload, pkt.payload_size);
      outEncodedFrame->setHeader(inFrame->tag, inFrame->seq, inFrame->ts, pkt.payload_size);

      portManager.sendOutput("out_encoded_frame", outEncodedFrame);
      double et = getTsNow();
      debug_print("encodeTime(%lf), sentSize(%d)", et-st, outEncodedFrame->dataSize);
      if(logger.isSet()) logger.getInstance()->info("encodingTime/KernelExeTime/Sent Size\t{}\t{}\t{}",
          enct-st, et-st, outEncodedFrame->dataSize);


      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);
      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

