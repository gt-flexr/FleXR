macro(kernel_nvmpi_decoder)
  if(${KERNEL_NVMPI_DECODER})

    if(${USE_NVMPI_PORTABLE})
      add_definitions(-D__FLEXR_KERNEL_NVMPI_DECODER__)
      list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_NVMPI_DECODER__)
      message("\t [Intermediate] NvmpiDecoder")
    else()
      message(WARNING "NvmpiDecoder is not enabled without USE_NVMPI_PORTABLE")
    endif()

  endif()
endmacro()

