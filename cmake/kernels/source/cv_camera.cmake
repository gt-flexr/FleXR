macro(kernel_cv_camera)
  if(${KERNEL_CV_CAMERA})
    add_definitions(-D__FLEXR_KERNEL_CV_CAMERA__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_CV_CAMERA__)
    message("\t [Source] CVCamera")
  endif()
endmacro()

