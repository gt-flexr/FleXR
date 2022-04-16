#pragma once

#ifdef __FLEXR_KERNEL_RGBA2RGB_APP_SOURCE__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "flexr_core/include/core.h"
#include "flexr_kernels/include/source/rgba2rgb_app_source.h"

namespace flexr
{
  namespace yaml
  {

    class YamlRgba2RgbAppSource: public YamlFleXRKernel
    {
      public:
        std::string shmqName;
        int maxShmqElem;
        int width, height;

        YamlRgba2RgbAppSource();

        void parseRgba2RgbAppSource(const YAML::Node &node);
        void parseRgba2RgbAppSourceSpecific(const YAML::Node &node);
        void printRgba2RgbAppSource();
        void printRgba2RgbAppSourceSpecific();

        void* make();
    };
  }
}

#endif

