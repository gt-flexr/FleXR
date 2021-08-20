file(GLOB_RECURSE FLEXR_SOURCE_KERNEL_SRCS *.cc)

add_library(flexr_source_kernel ${FLEXR_SOURCE_KERNEL_SRCS})
target_include_directories(flexr_source_kernel PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/flexrinc")

target_compile_options(flexr_source_kernel PUBLIC ${FLEXR_CXX_FLAGS})
target_link_libraries(flexr_source_kernel ${FLEXR_LINKER_FLAGS})
target_link_libraries(flexr_source_kernel ${FLEXR_LINKER_LIBS})
target_link_libraries(flexr_source_kernel flexr_component)

install(TARGETS flexr_source_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

