#ifdef __FLEXR_KERNEL_CV2GL_RGB_DISPLAY__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    Cv2GlRgbDisplay::Cv2GlRgbDisplay(std::string id, int width, int height): FleXRKernel(id), width(width), height(height)
    {
      setName("Cv2GlRgbDisplay");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);

      isInit = false;
    }


    Cv2GlRgbDisplay::~Cv2GlRgbDisplay()
    {
      glfwDestroyWindow(glWindow);
      glfwTerminate();
    }


    raft::kstatus Cv2GlRgbDisplay::run()
    {
      if(isInit == false)
      {
        glfwSetErrorCallback(gl_utils::error_callback);
        if (!glfwInit()) {
          exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glWindow = glfwCreateWindow(width, height, "FleXR Cv2GlRgbDisplay", NULL, NULL);
        if (!glWindow) {
          glfwTerminate();
          exit(EXIT_FAILURE);
        }

        glfwSetKeyCallback(glWindow, gl_utils::key_callback);
        glfwSetWindowSizeCallback(glWindow, gl_utils::resize_callback);

        glfwMakeContextCurrent(glWindow);

        glfwSwapInterval(1);

        //  Initialise glew (must occur AFTER window creation or glew will error)
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
          debug_print("GLEW initialisation error: %s", glewGetErrorString(err));
          exit(-1);
        }
        debug_print("GLEW initialisation with version %s", glewGetString(GLEW_VERSION));

        gl_utils::init_opengl(width, height);
        isInit = true;
      }

      Cv2GlRgbDisplayMsgType *inFrame = portManager.getInput<Cv2GlRgbDisplayMsgType>("in_frame");

      double st = getTsNow();
      if(!glfwWindowShouldClose(glWindow))
      {
        cv::Mat frame = inFrame->data.useAsCVMat();
        //cv::flip(frame, frame, 0);
        gl_utils::drawBgrCvToRgbGlWnd(frame, width, height);
        double et = getTsNow();

        glfwSwapBuffers(glWindow);
        glfwPollEvents();

        if(logger.isSet()) logger.getInstance()->info("{} frame disp_time/e2e_wo_disp/e2e_w_disp\t{}\t{}\t{}",
                                                      inFrame->seq, et - st, st - inFrame->ts, et - inFrame->ts);
        //debug_print("disp(%lf), e2e info: %s(%d:%d)", et-st, inFrame->tag, inFrame->seq, et-inFrame->ts);

      }

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);
      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

