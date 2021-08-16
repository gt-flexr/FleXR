#ifndef __FLEXR_YAML_RTP_FRAME_SENDER__
#define __FLEXR_YAML_RTP_FRAME_SENDER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "kernels/sink/rtp_frame_sender.h"

#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml RTPFrameSender kernel
     * @see flexr::kernels::RtpFrameSender
     */
    class YamlRtpFrameSender: public YamlFleXRKernel
    {
      public:
        std::string encoder;
        int width, height, bitrate;

        YamlRtpFrameSender();

        /**
         * @brief Parse RTP frame sender kernel info
         * @param node
         *  YAML node to parse
         */
        void parseRtpFrameSender(const YAML::Node &node);


        /**
         * @brief Parse specifics of RTP frame sender kernel
         * @param node
         *  YAML node to parse
         */
        void parseRtpFrameSenderSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed RTP frame sender info
         */
        void printRtpFrameSender();


        /**
         * @brief Print parsed RTP frame sender specifics
         */
        void printRtpFrameSenderSpecific();


        void* make();
    };
  }
}
#endif

