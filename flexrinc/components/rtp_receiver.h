#ifndef __FLEXR_COMPONENTS_RTP_RECEIVER__
#define __FLEXR_COMPONENTS_RTP_RECEIVER__

#include <uvgrtp/lib.hh>
#include "defs.h"
#include "types/types.h"

namespace flexr {
  namespace components {

    /**
     * @brief Component to receive RTP stream
     */
    class RTPReceiver
    {
    private:
      uvg_rtp::context rtpContext;
      uvg_rtp::session *rtpSession;
      uvg_rtp::media_stream *rtpStream;

      /**
       * @brief Unreference and delete auxiliary RTP frame information while keeping data
       * @param rtpFrame
       *  UvgRTP frame data to unref
       */

      void unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame);

    public:
      ~RTPReceiver();


      /**
       * @brief Initialize RTP session
       * @param addr
       *  Address for receiving
       * @param evenPort
       *  Port number for receiving
       */
      RTPReceiver(std::string addr, int evenPort);


      /**
       * @brief Receive a RTP stream of dynamically allocated message buffer
       * @param[out] outDataBuffer
       *  Pointer to the received data buffer
       * @param[out] outDataSize
       *  Size of the received data
       * @return Boolean of success or faile of receiving
       */
      bool receiveDynamic(uint8_t **outDataBuffer, uint32_t &outDataSize);


      /**
       * @brief Receive a RTP stream of dynamically allocated message buffer with tracking info
       * @param[out] outDataBuffer
       *  Pointer to the received data buffer
       * @param[out] outDataSize
       *  Size of the received data
       * @param[out] outTag
       *  Tag for tracking
       * @param[out] outSeq
       *  Message sequence for tracking
       * @param[out] outTs
       *  Timestamp for tracking
       * @return Boolean of success or faile of receiving
       */
      bool receiveDynamicWithTrackingInfo(uint8_t **outDataBuffer, uint32_t &outDataSize,
                                         char *outTag, uint32_t &outSeq, double &outTs);


      /**
       * @brief Receive a RTP stream of static messages
       * @param[in] inDatasize
       *  Size of data to receive
       * @param[out] outReceivedData
       *  Pointer to the data buffer for receiving
       * @return Boolean of success or faile of receiving
       */
      bool receiveStatic(uint32_t inDataSize, void *outReceivedData);


      /**
       * @brief Receive a RTP stream of static messages with tracking info
       * @param[in] inDatasize
       *  Size of data to receive
       * @param[out] outReceivedData
       *  Pointer to the data buffer for receiving
       * @param[out] outTag
       *  Tag for tracking
       * @param[out] outSeq
       *  Message sequence for tracking
       * @param[out] outTs
       *  Timestamp for tracking
       * @return Boolean of success or faile of receiving
       */
      bool receiveStaticWithTrackingInfo(uint32_t inDataSize, uint8_t *outReceivedData,
                                         char *outTag, uint32_t &outSeq, double &outTs);

    };
  }
}

#endif

