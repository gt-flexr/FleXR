#ifndef __FLEXR_YAML_RTP_FRAME_RECEIVER__
#define __FLEXR_YAML_RTP_FRAME_RECEIVER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>
#include "defs.h"
#include "yaml/yaml_flexr_kernel.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml RTPFrameReceiver kernel
     * @see flexr::kernels::RTPFrameReceiver
     */
    class YamlRtpFrameReceiver: public YamlFleXRKernel
    {
      public:
        std::string decoder;
        int width, height;

        YamlRtpFrameReceiver();

        /**
         * @brief Parse RTPFrameReceiver kernel info
         * @param node
         *  YAML node to parse
         */
        void parseRtpFrameReceiver(const YAML::Node &node);


        /**
         * @brief Parse specifics of RTPFrameReceiver kernel
         * @param node
         *  YAML node to parse
         */
        void parseRtpFrameReceiverSpecific(const YAML::Node &node);


        /**
         * @brief Print parsed RTPFrameReceiver info
         */
        void printRtpFrameReceiver();


        /**
         * @brief Print parsed RTPFrameReceiver specifics
         */
        void printRtpFrameReceiverSpecific();
    };
  }
}

#endif

