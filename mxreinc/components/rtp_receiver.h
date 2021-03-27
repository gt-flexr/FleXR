#ifndef __MXRE_COMPONENTS_RTP_RECEIVER__
#define __MXRE_COMPONENTS_RTP_RECEIVER__

#include <uvgrtp/lib.hh>
#include "defs.h"
#include "types/types.h"

namespace mxre {
  namespace components {
    class RTPReceiver {
    private:
      uvg_rtp::context rtpContext;
      uvg_rtp::session *rtpSession;
      uvg_rtp::media_stream *rtpStream;

    public:
      ~RTPReceiver();
      RTPReceiver(std::string ip, int even_port);
      bool receiveDynamic(uint8_t **outDataBuffer, uint32_t *outDataSize);
      bool receiveDynamicWithTrackinInfo(uint8_t **outDataBuffer, uint32_t *outDataSize,
                                         uint32_t *outIndex, double *outTimestamp);
      bool receiveStatic(uint32_t inDataSize, void *outReceivedData);
      bool receiveStaticWithTrackinInfo(uint32_t inDataSize, uint8_t *outReceivedData,
                                        uint32_t *outIndex, double *outTimestamp);

      void unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame);
    };
  }
}
#endif

