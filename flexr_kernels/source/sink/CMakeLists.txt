file(GLOB_RECURSE FLEXR_SINK_KERNEL_SRCS *.cc)

add_library(flexr_sink_kernel SHARED ${FLEXR_SINK_KERNEL_SRCS})
target_include_directories(flexr_sink_kernel PUBLIC "${PROJECT_SOURCE_DIR}")

target_compile_options(flexr_sink_kernel PUBLIC ${FLEXR_KERNEL_CXX_FLAGS})
target_link_libraries(flexr_sink_kernel ${FLEXR_KERNEL_LINKER_FLAGS})
target_link_libraries(flexr_sink_kernel ${FLEXR_KERNEL_LINKER_LIBS})
target_link_libraries(flexr_sink_kernel flexr_core)

install(TARGETS flexr_sink_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

