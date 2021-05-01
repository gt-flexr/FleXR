file(GLOB_RECURSE MXRE_INTERMEDIATE_KERNEL_SRCS *.cc)

add_library(mxre_intermediate_kernel ${MXRE_INTERMEDIATE_KERNEL_SRCS})
target_include_directories(mxre_intermediate_kernel PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/mxreinc")

target_compile_options(mxre_intermediate_kernel PUBLIC ${MXRE_CXX_FLAGS})
target_link_libraries(mxre_intermediate_kernel ${MXRE_LINKER_FLAGS})
target_link_libraries(mxre_intermediate_kernel ${MXRE_LINKER_LIBS})
target_link_libraries(mxre_intermediate_kernel mxre_type)
target_link_libraries(mxre_intermediate_kernel mxre_component)

install(TARGETS mxre_intermediate_kernel ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

