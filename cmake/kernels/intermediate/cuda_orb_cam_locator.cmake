macro(kernel_cuda_orb_cam_locator)
  if(${KERNEL_CUDA_ORB_CAM_LOCATOR})
    add_definitions(-D__FLEXR_KERNEL_CUDA_ORB_CAM_LOCATOR__)
    message("\t [Intermediate] CudaOrbCamLocator")
  endif()
endmacro()

