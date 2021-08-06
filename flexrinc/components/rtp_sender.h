#ifndef __FLEXR_COMPONENTS_RTP_SENDER__
#define __FLEXR_COMPONENTS_RTP_SENDER__

#include <uvgrtp/lib.hh>
#include "defs.h"
#include "types/types.h"

namespace flexr {
  namespace components {
    /**
     * @brief Component to send RTP stream
     */
    class RTPSender {
    private:
      uvg_rtp::context rtpContext;
      uvg_rtp::session *rtpSession;
      uvg_rtp::media_stream *rtpFragmentingStream;
      uvg_rtp::media_stream *rtpTinyStream;

    public:
      ~RTPSender();


      /**
       * @brief Initialize RTP session
       * @param addr
       *  Destination address for sending
       * @param evenPort
       *  Destination node port for sending
       */
      RTPSender(std::string addr, int evenPort);


      /**
       * @brief Send data as RTP stream
       * @param inData
       *  Pointer to the input data to send
       * @param inDataSize
       *  Size of the data to send
       * @return Boolean of sending success and fail
       */
      bool send(uint8_t *inData, uint32_t inDataSize);


      /**
       * @brief Send data with tracking info as RTP stream
       * @param inData
       *  Pointer to the input data to send
       * @param inDataSize
       *  Size of the data to send
       * @param tag
       *  Message tag for tacking
       * @param seq
       *  Sequence number of the sending message
       * @param ts
       *  Timestamp of the sending message
       * @return Boolean of sending success and fail
       */
      bool sendWithTrackingInfo(uint8_t *inData, uint32_t inDataSize, char *tag, uint32_t seq, double ts);
    };
  }
}
#endif

