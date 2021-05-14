#include <components/rtp_receiver.h>
#include <bits/stdc++.h>

namespace mxre {
  namespace components
  {
    RTPReceiver::~RTPReceiver()
    {
      rtpSession->destroy_stream(rtpStream);
      rtpContext.destroy_session(rtpSession);
    }

    RTPReceiver::RTPReceiver(std::string ip, int even_port)
    {
      debug_print("Receive RTP stream from %s:%d", ip.c_str(), even_port);
      rtpSession = rtpContext.create_session(ip);
      rtpStream = rtpSession->create_stream(even_port, -1, RTP_FORMAT_GENERIC, RCE_FRAGMENT_GENERIC);
    }

    bool RTPReceiver::receiveDynamic(uint8_t **outDataBuffer, uint32_t &outDataSize)
    {
      uvg_rtp::frame::rtp_frame *rtpFrame = nullptr;
      if(*outDataBuffer) { delete *outDataBuffer; *outDataBuffer = nullptr; }

      rtpFrame = rtpStream->pull_frame();
      if(rtpFrame != nullptr) {
        outDataSize = rtpFrame->payload_len;
        *outDataBuffer = rtpFrame->payload;
        unrefFrameExceptData(rtpFrame);

        debug_print("Receiving data size %d at %p", outDataSize, (void*)*outDataBuffer);
        return true;
      }

      return false;
    }


    bool RTPReceiver::receiveDynamicWithTrackingInfo(uint8_t **outDataBuffer, uint32_t &outDataSize,
                                                     char *outTag, uint32_t &outSeq, double &outTs)
    {
      types::RTPTrackingInfo trackingInfo;
      // RTP message: | trackinInfo |   Data   |
      //   - receive trackinInfo & 2-level sanity check
      //     - size of tracking info
      //     - invoice of tracking info
      //   - if tracking info is fine, receive data
      if(receiveStatic(sizeof(types::RTPTrackingInfo), &trackingInfo)) {
        if(trackingInfo.invoice == MXRE_RTP_TRACKING_INVOICE) {
          strcpy(outTag, trackingInfo.tag);
          outSeq = trackingInfo.seq;
          outTs = trackingInfo.ts;
          debug_print("Data size to receive: %d", trackingInfo.dataSize);
          if(receiveDynamic(outDataBuffer, outDataSize)) {
            if(trackingInfo.dataSize == outDataSize) {
              debug_print("valid %s: seq(%d) ts(%f) size(%d)", outTag, outSeq, outTs, outDataSize);
              return true;
            }
            debug_print("invalid %s: seq(%d) ts(%f) sizeToRecv(%d)/realSize(%d)", outTag, outSeq, outTs,
                                                                                  trackingInfo.dataSize, outDataSize);
            return false;
          }
        }
      }
      return false;
    }


    bool RTPReceiver::receiveStatic(uint32_t inDataSize, void *outReceivedData) {
      uvg_rtp::frame::rtp_frame *rtpFrame = nullptr;

      rtpFrame = rtpStream->pull_frame();
      if(rtpFrame != nullptr) {
        if(inDataSize == rtpFrame->payload_len) {
          memcpy(outReceivedData, rtpFrame->payload, inDataSize);
          (void)uvg_rtp::frame::dealloc_frame(rtpFrame);
          return true;
        }
        else {
          (void)uvg_rtp::frame::dealloc_frame(rtpFrame);
          return false;
        }
      }
      return false;
    }


    bool RTPReceiver::receiveStaticWithTrackingInfo(uint32_t inDataSize, uint8_t *outReceivedData,
                                                   char *outTag, uint32_t &outSeq, double &outTs)
    {
      types::RTPTrackingInfo trackingInfo;
      // RTP message: | trackinInfo |   Data   |
      //   - receive trackinInfo & 2-level sanity check
      //     - size of tracking info
      //     - invoice of tracking info
      //   - if tracking info is fine, receive data
      if(receiveStatic(sizeof(types::RTPTrackingInfo), &trackingInfo)) {
        if(trackingInfo.invoice == MXRE_RTP_TRACKING_INVOICE) {
          strcpy(outTag, trackingInfo.tag);
          outSeq = trackingInfo.seq;
          outTs = trackingInfo.ts;
          if(receiveStatic(inDataSize, outReceivedData)) return true;
        }
      }
      return false;
    }


    void RTPReceiver::unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame) {
      if (rtpFrame->csrc) delete rtpFrame->csrc;
      if (rtpFrame->ext) delete rtpFrame->ext;
      if (rtpFrame->probation) delete rtpFrame->probation;
    }
  }
}
