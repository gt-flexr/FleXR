macro(kernel_bag_camera)
  if(${KERNEL_BAG_CAMERA})
    add_definitions(-D__FLEXR_KERNEL_BAG_CAMERA__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_BAG_CAMERA__)
    message("\t [Source] BagCamera")
  endif()
endmacro()

