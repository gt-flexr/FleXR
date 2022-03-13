macro(kernel_orb_cam_locator)
  if(${KERNEL_ORB_CAM_LOCATOR})
    add_definitions(-D__FLEXR_KERNEL_ORB_CAM_LOCATOR__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_ORB_CAM_LOCATOR__)
    message("\t [Intermediate] OrbCamLocator")
  endif()
endmacro()

