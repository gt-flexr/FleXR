file(GLOB_RECURSE FLEXR_YAML_SOURCE_KERNEL_SRCS *.cc)

add_library(flexr_yaml_source_kernel ${FLEXR_YAML_SOURCE_KERNEL_SRCS})
target_include_directories(flexr_yaml_source_kernel PUBLIC "${PROJECT_SOURCE_DIR}")

target_link_libraries(flexr_yaml_source_kernel flexr_core)
target_link_libraries(flexr_yaml_source_kernel flexr_kernel)

install(TARGETS flexr_yaml_source_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

