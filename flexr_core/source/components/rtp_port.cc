#include <bits/stdc++.h>
#include <flexr_core/include/components/rtp_port.h>

namespace flexr
{
  namespace components
  {

    RtpPort::~RtpPort()
    {
      rtpSession->destroy_stream(stream);
      rtpContext.destroy_session(rtpSession);
    }


    RtpPort::RtpPort()
    {
    }


    RtpPort::RtpPort(std::string addr, int recvEvenPort, int sendEvenPort)
    {
      init(addr, recvEvenPort, sendEvenPort);
    }


    void RtpPort::init(std::string addr, int recvEvenPort, int sendEvenPort)
    {
      rtpSession = rtpContext.create_session(addr);
      unsigned flags = RCE_NO_SYSTEM_CALL_CLUSTERING | RCE_FRAGMENT_GENERIC;
      stream = rtpSession->create_stream(recvEvenPort, sendEvenPort, RTP_FORMAT_GENERIC, flags);
      stream->configure_ctx(RCC_UDP_SND_BUF_SIZE, 20 * 1000 * 1000);
      stream->configure_ctx(RCC_UDP_RCV_BUF_SIZE, 20 * 1000 * 1000);
      stream->configure_ctx(RCC_PKT_MAX_DELAY,                 200); // Relaxed lossy recv
    }


    bool RtpPort::send(uint8_t *inData, uint32_t inDataSize, uint32_t ts)
    {
      if (stream->push_frame(inData, inDataSize, ts, RTP_NO_FLAGS) != RTP_OK) {
        debug_print("RTP push_frame failed..");
        return false;
      }
      return true;
    }


    bool RtpPort::receiveMsg(bool inBlock, uint8_t* &outBuffer, uint32_t &outSize, uint32_t &outTs)
    {
      uvg_rtp::frame::rtp_frame *rtpFrame = nullptr;
      if(outBuffer != nullptr) { delete outBuffer; outBuffer = nullptr; }

      if(inBlock)
        rtpFrame = stream->pull_frame();
      else
        rtpFrame = stream->pull_frame(1);

      if(rtpFrame != nullptr) {
        outTs = rtpFrame->header.timestamp;
        outSize = rtpFrame->payload_len;
        outBuffer = std::move(rtpFrame->payload);
        debug_print("Received RTP msg: size(%ld), ts(%d)", rtpFrame->payload_len, outTs);

        unrefFrameExceptData(rtpFrame);
        return true;
      }
      return false;
    }


    void RtpPort::unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame) {
      if (rtpFrame->csrc) delete rtpFrame->csrc;
      if (rtpFrame->ext) delete rtpFrame->ext;
      if (rtpFrame->probation) delete rtpFrame->probation;
    }

  }
}

