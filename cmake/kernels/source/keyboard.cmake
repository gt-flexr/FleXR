macro(kernel_keyboard)
  if(${KERNEL_KEYBOARD})
    add_definitions(-D__FLEXR_KERNEL_KEYBOARD__)
    message("\t [Source] Keyboard")
  endif()
endmacro()

