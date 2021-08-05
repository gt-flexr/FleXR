#ifndef __FLEXR_COMPONENTS_RTP_SENDER__
#define __FLEXR_COMPONENTS_RTP_SENDER__

#include <uvgrtp/lib.hh>
#include "defs.h"
#include "types/types.h"

namespace flexr {
  namespace components {
    class RTPSender {
    private:
      uvg_rtp::context rtpContext;
      uvg_rtp::session *rtpSession;
      uvg_rtp::media_stream *rtpFragmentingStream;
      uvg_rtp::media_stream *rtpTinyStream;

    public:
      ~RTPSender();
      RTPSender(std::string dest, int even_port);
      bool send(uint8_t *inData, uint32_t inDataSize);
      bool sendWithTrackingInfo(uint8_t *inData, uint32_t inDataSize,
                                char *tag, uint32_t seq, double ts);
    };
  }
}
#endif

