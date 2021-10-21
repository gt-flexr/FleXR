add_library(flexr_kernel INTERFACE)

add_subdirectory(source)
add_subdirectory(intermediate)
add_subdirectory(sink)

target_link_libraries(flexr_kernel INTERFACE flexr_source_kernel)
target_link_libraries(flexr_kernel INTERFACE flexr_sink_kernel)
target_link_libraries(flexr_kernel INTERFACE flexr_intermediate_kernel)

add_subdirectory(yaml)

