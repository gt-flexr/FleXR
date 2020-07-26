file(GLOB_RECURSE MXRE_SRCS *.cc)

add_library(mxre ${MXRE_SRCS})
target_include_directories(mxre PUBLIC ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR})

target_compile_options(mxre PUBLIC "-W" "-Wall" "-Wextra" "-Wpedantic")
