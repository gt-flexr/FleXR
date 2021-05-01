file(GLOB_RECURSE MXRE_SOURCE_KERNEL_SRCS *.cc)

add_library(mxre_source_kernel ${MXRE_SOURCE_KERNEL_SRCS})
target_include_directories(mxre_source_kernel PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/mxreinc")

target_compile_options(mxre_source_kernel PUBLIC ${MXRE_CXX_FLAGS})
target_link_libraries(mxre_source_kernel ${MXRE_LINKER_FLAGS})
target_link_libraries(mxre_source_kernel ${MXRE_LINKER_LIBS})
target_link_libraries(mxre_source_kernel mxre_component)

install(TARGETS mxre_source_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

