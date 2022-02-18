macro(kernel_nvmpi_encoder)
  if(${KERNEL_NVMPI_ENCODER})
    add_definitions(-D__FLEXR_KERNEL_NVMPI_ENCODER__)
    message("\t [Intermediate] NvmpiEncoder")
  endif()
endmacro()

