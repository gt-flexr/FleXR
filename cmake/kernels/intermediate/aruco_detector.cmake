macro(kernel_aruco_detector)
  if(${KERNEL_ARUCO_DETECTOR})
    add_definitions(-D__FLEXR_KERNEL_ARUCO_DETECTOR__)
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS -D__FLEXR_KERNEL_ARUCO_DETECTOR__)
    message("\t [Intermediate] ArUcoDetector")
  endif()
endmacro()

