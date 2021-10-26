macro(kernel_sample_marker_renderer)
  if(${KERNEL_SAMPLE_MARKER_RENDERER})
    add_definitions(-D__FLEXR_KERNEL_SAMPLE_MARKER_RENDERER__)
    message("\t [Intermediate] SampleMarkerRenderer")
  endif()
endmacro()

