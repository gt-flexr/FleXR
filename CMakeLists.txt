cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(MXRE CXX)
set(CMAKE_CXX_COMPILER g++)
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
file(GLOB_RECURSE MXRE_SRCS src/*.cc)
file(GLOB_RECURSE TEST_SRCS tests/*.cc)
include_directories("include")

# MXRE compile info
set(MXRE_CXX_FLAGS "")
set(MXRE_LINKER_FLAGS "")
set(MXRE_LINKER_LIBS "")

# Include cmake utils
include(cmake/utils/AddTestExecutables.cmake)

# Include cmake modules
include(cmake/modules/OpenGL.cmake)
include(cmake/modules/OpenCV.cmake)
include(cmake/modules/RaftLib.cmake)

use_opencv()
use_opengl()
use_raftlib()

message(STATUS "[MXRE COMPILE INFO]")
message("\t MXRE_CXX_FLAGS: ${MXRE_CXX_FLAGS}")
message("\t MXRE_LINKER_FLAGS: ${MXRE_LINKER_FLAGS}")
message("\t MXRE_LINKER_LIBS: ${MXRE_LINKER_LIBS}")
message("\t MXRE_SRCS: ${MXRE_SRCS}")

add_test_executables("${TEST_SRCS}")

