macro(kernel_frame_decoder)
  if(${KERNEL_FRAME_DECODER})
    add_definitions(-D__FLEXR_KERNEL_FRAME_DECODER__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_FRAME_DECODER__)
    message("\t [Intermediate] FrameDecoder")
  endif()
endmacro()

