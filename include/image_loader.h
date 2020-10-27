#ifndef __MXRE_IMAGE_LOADER__
#define __MXRE_IMAGE_LOADER__

#include <bits/stdc++.h>
#include <raft>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"

namespace mxre
{
  namespace pipeline
  {
    namespace input_srcs
    {
      class ImageLoader: public raft::kernel
      {
      private:
        unsigned int frame_idx;
        int maxPlaceValue;
        std::string path, stemName;
        int width, height;

      public:
        ImageLoader(std::string path="", std::string stemName="", int startIndex=0, int maxPlaceValue=0,
            int width=1280, int height=720);
        ~ImageLoader();
        virtual raft::kstatus run();
      };
    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre

#endif

