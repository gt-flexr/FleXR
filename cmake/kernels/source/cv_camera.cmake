macro(kernel_cv_camera)
  if(${KERNEL_CV_CAMERA})
    add_definitions(-D__FLEXR_KERNEL_CV_CAMERA__)
    message("\t [Source] CVCamera")
  endif()
endmacro()

