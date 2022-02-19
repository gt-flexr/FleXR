#pragma once

#ifdef __FLEXR_KERNEL_CV2GL_RGB_DISPLAY__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "flexr_core/include/core.h"
#include "flexr_core/include/utils/gl_utils.h"

namespace flexr
{
  namespace kernels
  {

    using Cv2GlRgbDisplayMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to display frames
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * in_frame       | @ref flexr::types::Message<@ref flexr::types::Frame>
     */
    class Cv2GlRgbDisplay : public FleXRKernel
    {
    public:
      int width, height;
      GLFWwindow *glWindow;
      bool isInit;

      /**
       * @brief Initialize display kernel
       * @param id
       *  Kernel ID
       * @param width
       *  display width
       * @param height
       *  display height
       */
      Cv2GlRgbDisplay(std::string id, int width, int height);

      ~Cv2GlRgbDisplay();

      raft::kstatus run() override;
    };

  }   // namespace kernels
} // namespace flexr

#endif

