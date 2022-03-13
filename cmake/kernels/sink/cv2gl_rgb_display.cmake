macro(kernel_cv2gl_rgb_display)
  if(${KERNEL_CV2GL_RGB_DISPLAY})
    add_definitions(-D__FLEXR_KERNEL_CV2GL_RGB_DISPLAY__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_CV2GL_RGB_DISPLAY__)
    message("\t [Sink] Cv2GlRgbDisplay")
  endif()
endmacro()

