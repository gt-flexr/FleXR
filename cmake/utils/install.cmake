macro(util_set_installation)
  # Install
  string(REPLACE ";" " " FLEXR_CORE_CXX_FLAGS    "${FLEXR_CORE_CXX_FLAGS}")
  string(REPLACE ";" " " FLEXR_CORE_LINKER_FLAGS "${FLEXR_CORE_LINKER_FLAGS}")
  string(REPLACE ";" " " FLEXR_CORE_LINKER_LIBS  "${FLEXR_CORE_LINKER_LIBS}")

  string(REPLACE ";" " " FLEXR_KERNEL_CXX_FLAGS    "${FLEXR_KERNEL_CXX_FLAGS}")
  string(REPLACE ";" " " FLEXR_KERNEL_LINKER_FLAGS "${FLEXR_KERNEL_LINKER_FLAGS}")
  string(REPLACE ";" " " FLEXR_KERNEL_LINKER_LIBS  "${FLEXR_KERNEL_LINKER_LIBS}")

  set(FLEXR_HEADER flexr)
  install(FILES ${CMAKE_SOURCE_DIR}/${FLEXR_HEADER} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

  if(NOT WIN32)
    if(NOT DEFINED ENV{PKG_CONFIG_PATH})
      set( PKG_CONFIG_PATH "/usr/local/lib/pkgconfig")
      message("PKG_CONFIG_PATH is not found. Set it with ${PKG_CONFIG_PATH}")
    endif(NOT DEFINED ENV{PKG_CONFIG_PATH})

    configure_file("flexr.pc.in" "flexr.pc" @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/flexr.pc DESTINATION ${PKG_CONFIG_PATH}/)
  endif(NOT WIN32)
endmacro()

