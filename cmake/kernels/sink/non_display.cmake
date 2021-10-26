macro(kernel_non_display)
  if(${KERNEL_NON_DISPLAY})
    add_definitions(-D__FLEXR_KERNEL_NON_DISPLAY__)
    message("\t [Sink] NonDisplay")
  endif()
endmacro()

