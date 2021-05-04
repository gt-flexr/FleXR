#ifndef __MXRE_COMPONENTS_RTP_RECEIVER__
#define __MXRE_COMPONENTS_RTP_RECEIVER__

#include <uvgrtp/lib.hh>
#include "defs.h"
#include "types/types.h"

namespace mxre {
  namespace components {
    class RTPReceiver
    {
    private:
      uvg_rtp::context rtpContext;
      uvg_rtp::session *rtpSession;
      uvg_rtp::media_stream *rtpStream;

    public:
      ~RTPReceiver();
      RTPReceiver(std::string ip, int even_port);
      bool receiveDynamic(uint8_t **outDataBuffer, uint32_t &outDataSize);
      bool receiveDynamicWithTrackingInfo(uint8_t **outDataBuffer, uint32_t &outDataSize,
                                         std::string &outTag, uint32_t &outSeq, double &outTs);
      bool receiveStatic(uint32_t inDataSize, void *outReceivedData);
      bool receiveStaticWithTrackingInfo(uint32_t inDataSize, uint8_t *outReceivedData,
                                        std::string &outTag, uint32_t &outSeq, double &outTs);

      void unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame);
    };
  }
}
#endif

