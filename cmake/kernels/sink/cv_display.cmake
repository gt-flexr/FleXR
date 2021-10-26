macro(kernel_cv_display)
  if(${KERNEL_CV_DISPLAY})
    add_definitions(-D__FLEXR_KERNEL_CV_DISPLAY__)
    message("\t [Sink] CVDisplay")
  endif()
endmacro()

