macro(kernel_keyboard)
  if(${KERNEL_KEYBOARD})
    add_definitions(-D__FLEXR_KERNEL_KEYBOARD__)
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS -D__FLEXR_KERNEL_KEYBOARD__)
    message("\t [Source] Keyboard")

    pkg_search_module(X11 REQUIRED x11)
    if(X11_FOUND)
      message("\t\t X11_LDFLAGS: ${X11_LDFLAGS}")
      list(APPEND FLEXR_KERNEL_LINKER_FLAGS ${X11_LDFLAGS})
    endif(X11_FOUND)

  endif()
endmacro()

