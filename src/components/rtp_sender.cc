#include <components/rtp_sender.h>
#include <bits/stdc++.h>

namespace mxre {
  namespace components {
    RTPSender::~RTPSender() {
      rtpSession->destroy_stream(rtpFragmentingStream);
      rtpSession->destroy_stream(rtpTinyStream);
      rtpContext.destroy_session(rtpSession);
    }

    RTPSender::RTPSender(std::string dest, int even_port)
    {
      rtpSession = rtpContext.create_session(dest);
      rtpFragmentingStream = rtpSession->create_stream(-1, even_port, RTP_FORMAT_GENERIC, RCE_FRAGMENT_GENERIC);
      rtpTinyStream = rtpSession->create_stream(-1, even_port, RTP_FORMAT_GENERIC, 0);
    }

    bool RTPSender::send(uint8_t *inData, uint32_t inDataSize) {
      debug_print("Sending data size %d", inDataSize);
      if(inDataSize > MXRE_PAYLOAD_UNIT_SIZE) {
        if (rtpFragmentingStream->push_frame(inData, inDataSize, RTP_SLICE) != RTP_OK) {
          debug_print("RTP push_frame failed..");
          return false;
        }
      }
      else {
        if (rtpTinyStream->push_frame(inData, inDataSize, RTP_SLICE) != RTP_OK) {
          debug_print("RTP push_frame failed..");
          return false;
        }
      }
      
      return true;
    }
  }
}