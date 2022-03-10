#pragma once

#ifdef __FLEXR_KERNEL_IMG_PLAYER__

#include <bits/stdc++.h>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using ImgPlayerMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to get images from a directory
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * out_frame      | @ref flexr::types::Message< @ref flexr::types::Frame>
     */
    class ImgPlayer: public FleXRKernel
    {
    private:
      std::ostringstream os;
      std::vector<cv::Mat> imgs;

      uint32_t seq;
      std::string imgPath;
      int fnZeros, nImgs;
      bool iterate, caching;

    public:
      /**
       * @brief Initialize bag camera kernel
       * @param id
       *  Kernel ID
       * @param imgPath
       *  Image file path to read
       * @param fnZeros
       *  Number of zero pads to image file name
       * @param fps
       *  Target frequency to generate frames
       * @param nImgs
       *  Number of images in path
       * @param iterate
       *  T/F for iterating
       * @param caching
       *  T/F for caching
       */
      ImgPlayer(std::string id, std::string imgPath, int fnZeros, int fps, int nImgs, bool iterate, bool caching);

      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace flexr

#endif

