macro(kernel_img_player)
  if(${KERNEL_IMG_PLAYER})
    add_definitions(-D__FLEXR_KERNEL_IMG_PLAYER__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_IMG_PLAYER__)
    message("\t [Source] ImgPlayer")
  endif()
endmacro()

