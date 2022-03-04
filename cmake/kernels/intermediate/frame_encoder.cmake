macro(kernel_frame_encoder)
  if(${KERNEL_FRAME_ENCODER})
    add_definitions(-D__FLEXR_KERNEL_FRAME_ENCODER__)
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS -D__FLEXR_KERNEL_FRAME_ENCODER__)
    message("\t [Intermediate] FrameEncoder")
  endif()
endmacro()

