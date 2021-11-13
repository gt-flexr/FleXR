#pragma once

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
      void unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame)
      {
        if (rtpFrame->csrc) delete rtpFrame->csrc;
        if (rtpFrame->ext) delete rtpFrame->ext;
        if (rtpFrame->probation) delete rtpFrame->probation;
      }


    public:
      ~RtpPort()
      {
        close();
      }


      /**
       * @brief Close RTP session
       */
      void close()
      {
        if(stream != nullptr)     rtpSession->destroy_stream(stream);
        if(rtpSession != nullptr) rtpContext.destroy_session(rtpSession);

        stream     = nullptr;
        rtpSession = nullptr;
      }


      RtpPort()
      {
        rtpSession = nullptr;
        stream     = nullptr;
      }


      /**
       * @brief Construct RTP session with initialization
       * @param addr
       *  Destination address
       * @param recvEvenPort
       *  Even port to recv
       * @param sendEvenPort
       *  Even port to send
       */
      RtpPort(std::string addr, int recvEvenPort, int sendEvenPort)
      {
        init(addr, recvEvenPort, sendEvenPort);
      }


      /**
       * @brief Initialize RTP session
       * @param addr
       *  Destination address
       * @param recvEvenPort
       *  Even port to recv
       * @param sendEvenPort
       *  Even port to send
       * @return Boolean of initialization success or failure
       */
      bool init(std::string addr, int recvEvenPort, int sendEvenPort)
      {
        rtpSession = rtpContext.create_session(addr);
        if(rtpSession == nullptr)
        {
          debug_print("fail to create rtpSession");
          return false;
        }

        unsigned flags = RCE_NO_SYSTEM_CALL_CLUSTERING | RCE_FRAGMENT_GENERIC;
        stream = rtpSession->create_stream(recvEvenPort, sendEvenPort, RTP_FORMAT_GENERIC, flags);
        if(stream == nullptr)
        {
          debug_print("fail to create rtpStream");
          return false;
        }

        stream->configure_ctx(RCC_UDP_SND_BUF_SIZE, 20 * 1000 * 1000);
        stream->configure_ctx(RCC_UDP_RCV_BUF_SIZE, 20 * 1000 * 1000);
        stream->configure_ctx(RCC_PKT_MAX_DELAY,                 200); // Relaxed lossy recv

        return true;
      }


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
      bool send(uint8_t *inData, uint32_t inDataSize, uint32_t ts)
      {
        if (stream->push_frame(inData, inDataSize, ts, RTP_NO_FLAGS) != RTP_OK) {
          debug_print("RTP push_frame failed..");
          return false;
        }
        return true;
      }


      /**
       * @brief Receive a RTP message with a dynamic buffer
       * @param[in] inBlock
       *  For blocking/non-blocking semantics
       * @param[out] outBuffer
       *  Reference of the pointer to the received data buffer
       * @param[out] outSize
       *  Size of the received data
       * @return Boolean of success or faile of receiving
       */
      bool receiveMsg(bool inBlock, uint8_t* &outBuffer, uint32_t &outSize)
      {
        uvg_rtp::frame::rtp_frame *rtpFrame = nullptr;
        if(outBuffer != nullptr) { delete outBuffer; outBuffer = nullptr; }

        if(inBlock)
          rtpFrame = stream->pull_frame();
        else
          rtpFrame = stream->pull_frame(1);

        if(rtpFrame != nullptr) {
          outSize = rtpFrame->payload_len;
          outBuffer = std::move(rtpFrame->payload);
          debug_print("Received RTP msg: size(%ld), ts(%d)", rtpFrame->payload_len, rtpFrame->header.timestamp);

          unrefFrameExceptData(rtpFrame);
          return true;
        }
        return false;
      }

    };
  }
}

