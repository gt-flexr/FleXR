macro(kernel_euroc_mono_inertial_reader)
  if(${KERNEL_EUROC_MONO_INERTIAL_READER})
    add_definitions(-D__FLEXR_KERNEL_EUROC_MONO_INERTIAL_READER__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_EUROC_MONO_INERTIAL_READER__)
    message("\t [Source] EurocMonoInertialReader")
  endif()
endmacro()

