add_library(mxre_kernel STATIC "")

add_subdirectory(source)
add_subdirectory(intermediate)
add_subdirectory(sink)

target_link_libraries(mxre_kernel mxre_source_kernel)
target_link_libraries(mxre_kernel mxre_sink_kernel)
target_link_libraries(mxre_kernel mxre_intermediate_kernel)

