file(GLOB_RECURSE MXRE_SRCS *.cc)

add_library(mxre ${MXRE_SRCS})
target_include_directories(mxre PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR})

target_compile_options(mxre PUBLIC "-W" "-Wall" "-Wextra" "-Wpedantic")
target_compile_options(mxre PUBLIC ${MXRE_CXX_FLAGS})
target_link_libraries(mxre ${MXRE_LINKER_FLAGS})
target_link_libraries(mxre ${MXRE_LINKER_LIBS})
