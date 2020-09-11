cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(MXRE CXX)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# DEBUG compile
#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
  add_definitions(-D__DEBUG__=1)
else()
  add_definitions(-D__DEBUG__=0)
endif()

# Profile compile
set(MXRE_PROFILE ON)
if(${MXRE_PROFILE})
  add_definitions(-D__PROFILE__=1)
else()
  add_definitions(-D__PROFILE__=0)
endif()


list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(PkgConfig REQUIRED)

if (UNIX)
  message(STATUS "[UNIX]")
  if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message("\t ${CMAKE_SYSTEM_NAME}")
  elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Apple")
    message("\t ${CMAKE_SYSTEM_NAME}")
  endif ()
endif()

# Source codes & headers
include_directories("include")

# MXRE compile info
set(MXRE_CXX_FLAGS "")
set(MXRE_LINKER_FLAGS "")
set(MXRE_LINKER_LIBS "")

# Include cmake modules
include(cmake/modules/OpenGL.cmake)
include(cmake/modules/OpenCV.cmake)
include(cmake/modules/RaftLib.cmake)
include(cmake/modules/LibZMQ.cmake)
include(cmake/modules/GStreamer.cmake)
include(cmake/modules/FFmpeg.cmake)

use_opencv()
use_opengl()
use_raftlib()
use_libzmq()
use_gstreamer()
use_ffmpeg()

message(STATUS "[MXRE COMPILE INFO]")
message("\t MXRE_CXX_FLAGS: ${MXRE_CXX_FLAGS}")
message("\t MXRE_LINKER_FLAGS: ${MXRE_LINKER_FLAGS}")
message("\t MXRE_LINKER_LIBS: ${MXRE_LINKER_LIBS}")
message("\t MXRE_SRCS: ${MXRE_SRCS}")

add_subdirectory(src)
add_subdirectory(tests)

