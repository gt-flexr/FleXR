file(GLOB_RECURSE FLEXR_YAML_INTERMEDIATE_KERNEL_SRCS *.cc)

add_library(flexr_yaml_intermediate_kernel ${FLEXR_YAML_INTERMEDIATE_KERNEL_SRCS})
target_include_directories(flexr_yaml_intermediate_kernel PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/flexrinc")

target_compile_options(flexr_yaml_intermediate_kernel PUBLIC ${FLEXR_CXX_FLAGS})
target_link_libraries(flexr_yaml_intermediate_kernel ${FLEXR_LINKER_FLAGS})
target_link_libraries(flexr_yaml_intermediate_kernel ${FLEXR_LINKER_LIBS})
#target_link_libraries(flexr_yaml_intermediate_kernel flexr_type)
#target_link_libraries(flexr_yaml_intermediate_kernel flexr_component)

install(TARGETS flexr_yaml_intermediate_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

