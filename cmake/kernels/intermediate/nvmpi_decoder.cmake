macro(kernel_nvmpi_decoder)
  if(${KERNEL_NVMPI_DECODER})
    add_definitions(-D__FLEXR_KERNEL_NVMPI_DECODER__)
    message("\t [Intermediate] NvmpiDecoder")
  endif()
endmacro()

