#pragma once

#ifdef __FLEXR_KERNEL_RGB2RGBA_APP_SINK__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/sink/rgb2rgba_app_sink.h"

namespace flexr
{
  namespace yaml
  {

    class YamlRgb2RgbaAppSink: public YamlFleXRKernel
    {
      public:
        std::string shmqName;
        int maxShmqElem;
        int width, height;

        YamlRgb2RgbaAppSink();

        void parseRgb2RgbaAppSink(const YAML::Node &node);
        void parseRgb2RgbaAppSinkSpecific(const YAML::Node &node);
        void printRgb2RgbaAppSink();
        void printRgb2RgbaAppSinkSpecific();

        void* make();
    };
  }
}
#endif

