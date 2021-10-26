macro(kernel_aruco_detector)
  if(${KERNEL_ARUCO_DETECTOR})
    add_definitions(-D__FLEXR_KERNEL_ARUCO_DETECTOR__)
    message("\t [Intermediate] ArUcoDetector")
  endif()
endmacro()

