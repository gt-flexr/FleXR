file(GLOB_RECURSE MXRE_TYPE_SRCS *.cc)

add_library(mxre_type ${MXRE_TYPE_SRCS})
target_include_directories(mxre_type PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/mxreinc"
                           "${PROJECT_SOURCE_DIR}/third-party"
                           "${CMAKE_BINARY_DIR}/config")

target_compile_options(mxre_type PUBLIC ${MXRE_CXX_FLAGS})
target_link_libraries(mxre_type ${MXRE_LINKER_FLAGS})
target_link_libraries(mxre_type ${MXRE_THIRD_PARTY})
target_link_libraries(mxre_type ${MXRE_LINKER_LIBS})

install(TARGETS mxre_type ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

