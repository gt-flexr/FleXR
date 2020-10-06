cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(MXRE CXX)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(cmake/utils/Util.cmake)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(PkgConfig REQUIRED)
# OS platform check
if (UNIX)
  message(STATUS "[UNIX]")
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message("\t ${CMAKE_SYSTEM_NAME}")
  elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Apple")
    message("\t ${CMAKE_SYSTEM_NAME}")
  endif ()
endif()

# Build options
build_option(DEBUG "Set Debug/Release" ON)
build_option(PROFILE "Set kernel profiling" ON)

mxre_option(USE_FFMPEG_NVCODEC "Use nvcodec with ffmpeg" OFF)
mxre_option(USE_OPENCV_CUDA "Use cuda-accelerated functions of OpenCV" OFF)

# Source codes & headers
configure_file(config/root_directory.h.in config/root_directory.h)

# MXRE compile info
set(MXRE_CXX_FLAGS "")
set(MXRE_LINKER_FLAGS "")
set(MXRE_LINKER_LIBS "")

# Include cmake modules
include_directories("include")
include(cmake/modules/OpenGL.cmake)
include(cmake/modules/OpenCV.cmake)
include(cmake/modules/RaftLib.cmake)
include(cmake/modules/LibZMQ.cmake)
include(cmake/modules/GStreamer.cmake)
include(cmake/modules/FFmpeg.cmake)

message(STATUS "[MXRE COMPILE INFO]")
message("\t MXRE_CXX_FLAGS: ${MXRE_CXX_FLAGS}")
message("\t MXRE_LINKER_FLAGS: ${MXRE_LINKER_FLAGS}")
message("\t MXRE_LINKER_LIBS: ${MXRE_LINKER_LIBS}")
message("\t MXRE_SRCS: ${MXRE_SRCS}")

add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(examples)

