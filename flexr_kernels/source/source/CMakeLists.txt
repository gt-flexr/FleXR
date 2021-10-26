file(GLOB_RECURSE FLEXR_SOURCE_KERNEL_SRCS *.cc)

add_library(flexr_source_kernel ${FLEXR_SOURCE_KERNEL_SRCS})
target_include_directories(flexr_source_kernel PUBLIC "${PROJECT_SOURCE_DIR}")

target_compile_options(flexr_source_kernel PUBLIC ${FLEXR_KERNEL_CXX_FLAGS})
target_link_libraries(flexr_source_kernel ${FLEXR_KERNEL_LINKER_FLAGS})
target_link_libraries(flexr_source_kernel ${FLEXR_KERNEL_LINKER_LIBS})
target_link_libraries(flexr_source_kernel flexr_core)

install(TARGETS flexr_source_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

