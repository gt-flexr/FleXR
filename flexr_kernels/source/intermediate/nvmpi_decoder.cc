#ifdef __FLEXR_KERNEL_NVMPI_DECODER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    /* Constructor() */
    NvmpiDecoder::NvmpiDecoder(std::string id, std::string decoderName, int width, int height):
      width(width), height(height), decoderName(decoderName),
      FleXRKernel(id)
    {
      setName("NvmpiDecoder");
      portManager.registerInPortTag ("in_encoded_frame", components::BLOCKING, utils::deserializeEncodedFrame);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, utils::serializeRawFrame);

      nvCodingType dec;
      if(decoderName == "h264_nvmpi") dec = NV_VIDEO_CodingH264;
      else if(decoderName == "hevc_nvmpi") dec = NV_VIDEO_CodingHEVC;
      decoder = nvpInitDecoder(dec);
    }


    /* Destructor() */
    NvmpiDecoder::~NvmpiDecoder()
    {
      nvpCloseDecoder(decoder);
    }


    /* Run() */
    raft::kstatus NvmpiDecoder::run()
    {
      DecoderOutFrameType *outFrame = portManager.getOutputPlaceholder<DecoderOutFrameType>("out_frame");

      int ret = 0;

      DecoderInEncodedFrameType *inEncodedFrame = portManager.getInput<DecoderInEncodedFrameType>("in_encoded_frame");
      outFrame->setHeader(inEncodedFrame->tag, inEncodedFrame->seq, inEncodedFrame->ts, inEncodedFrame->dataSize);
      outFrame->printHeader();

      double st = getTsNow();
      nvPacket pkt;
      pkt.payload      = inEncodedFrame->data;
      pkt.payload_size = inEncodedFrame->dataSize;
      nvpPutPktToDecoder(decoder, &pkt);
      cv::Mat decodedYuv = nvpGetDecodedYuvFrame(decoder, width, height);
      if(decodedYuv.rows != 1 && decodedYuv.cols != 1)
      {
        outFrame->data = types::Frame(height, width, CV_8UC3);
        cv::cvtColor(decodedYuv, outFrame->data.useAsCVMat(), cv::COLOR_YUV420p2RGB);
        outFrame->dataSize = outFrame->data.useAsCVMat().total() * outFrame->data.useAsCVMat().elemSize();
        double et = getTsNow();
        if(logger.isSet()) logger.getInstance()->info("st/et/decodingTime/recvSize\t{}\t {}\t {}\t {}",
            st, et, et-st, inEncodedFrame->dataSize);
        portManager.sendOutput("out_frame", outFrame);
      }

      delete inEncodedFrame->data;
      portManager.freeInput("in_encoded_frame", inEncodedFrame);
      freqManager.adjust();
      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

#endif

