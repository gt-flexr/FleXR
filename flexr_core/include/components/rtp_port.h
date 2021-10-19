#ifndef __FLEXR_CORE_COMPONENTS_RTP_PORT__
#define __FLEXR_CORE_COMPONENTS_RTP_PORT__

#include <uvgrtp/lib.hh>
#include "flexr_core/include/defs.h"
#include "flexr_core/include/types/types.h"

namespace flexr {
  namespace components {
    /**
     * @brief Component for remote port communication with RTP
     */
    class RtpPort {
    private:
      uvg_rtp::context rtpContext;
      uvg_rtp::session *rtpSession;
      uvg_rtp::media_stream *stream;

      /**
       * @brief Unreference and delete auxiliary RTP frame information while keeping data
       * @param rtpFrame
       *  UvgRTP frame data to unref
       */
      void unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame);

    public:
      ~RtpPort();
      RtpPort();

      /**
       * @brief Construct RTP session with initialization
       * @param addr
       *  Destination address
       * @param recvEvenPort
       *  Even port to recv
       * @param sendEvenPort
       *  Even port to send

       */
      RtpPort(std::string addr, int recvEvenPort, int sendEvenPort);


      /**
       * @brief Initialize RTP session
       * @param addr
       *  Destination address
       * @param recvEvenPort
       *  Even port to recv
       * @param sendEvenPort
       *  Even port to send
       */
      void init(std::string addr, int recvEvenPort, int sendEvenPort);


      /**
       * @brief Send data as RTP stream
       * @param inData
       *  Pointer to the input data to send
       * @param inDataSize
       *  Size of the data to send
       * @param ts
       *  Timestamp of the sending message
       * @return Boolean of sending success and fail
       */
      bool send(uint8_t *inData, uint32_t inDataSize, uint32_t ts);


      /**
       * @brief Receive a RTP message with a dynamic buffer
       * @param[in] inBlock
       *  For blocking/non-blocking semantics
       * @param[out] outBuffer
       *  Reference of the pointer to the received data buffer
       * @param[out] outSize
       *  Size of the received data
       * @param[out] outTs
       *  Timestamp for tracking
       * @return Boolean of success or faile of receiving
       */
      bool receiveMsg(bool inBlock, uint8_t* &outBuffer, uint32_t &outSize, uint32_t &outTs);

    };
  }
}
#endif

