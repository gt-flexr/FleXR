#include <components/rtp_receiver.h>
#include <bits/stdc++.h>

namespace mxre {
  namespace components {
    RTPReceiver::~RTPReceiver() {
      rtpSession->destroy_stream(rtpStream);
      rtpContext.destroy_session(rtpSession);
    }

    RTPReceiver::RTPReceiver(std::string ip, int even_port)
    {
      rtpSession = rtpContext.create_session(ip);
      rtpStream = rtpSession->create_stream(even_port, -1, RTP_FORMAT_GENERIC, RCE_FRAGMENT_GENERIC);
    }

    bool RTPReceiver::receive(uint8_t *outDataBuffer, uint32_t *outDataSize) {
      uvg_rtp::frame::rtp_frame *rtpFrame = nullptr;
      //if( (rtpFrame = rtpStream->pull_frame(20)) == nullptr ) return false;
      //
      rtpFrame = rtpStream->pull_frame();
      if(rtpFrame != nullptr) {
        *outDataSize = rtpFrame->payload_len;
        outDataBuffer = new uint8_t[*outDataSize];
        memcpy(outDataBuffer, rtpFrame->payload, *outDataSize);
        (void)uvg_rtp::frame::dealloc_frame(rtpFrame);
        debug_print("Receiving data size %d at %p", *outDataSize, (void*)outDataBuffer);
        return true;
      }

      return false;
    }
  }
}
