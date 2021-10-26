macro(kernel_frame_converter)
  if(${KERNEL_FRAME_CONVERTER})
    add_definitions(-D__FLEXR_KERNEL_FRAME_CONVERTER__)
    message("\t [Intermediate] FrameConverter")
  endif()
endmacro()

