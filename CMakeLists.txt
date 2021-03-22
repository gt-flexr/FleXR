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

mxre_option(USE_FFMPEG_NVCODEC "Use nvcodec with ffmpeg" ON)
mxre_option(USE_OPENCV_CUDA "Use cuda-accelerated functions of OpenCV" ON)

# Source codes & headers
configure_file(config/root_directory.h.in config/root_directory.h)

# MXRE compile info
set(MXRE_CXX_FLAGS "")
set(MXRE_LINKER_FLAGS "")
set(MXRE_LINKER_LIBS "")

# Include cmake modules
include_directories("mxreinc")
include(cmake/modules/OpenGL.cmake)
include(cmake/modules/OpenCV.cmake)
include(cmake/modules/RaftLib.cmake)
include(cmake/modules/LibZMQ.cmake)
include(cmake/modules/GStreamer.cmake)
include(cmake/modules/FFmpeg.cmake)
include(cmake/modules/Spdlog.cmake)
include(cmake/modules/yaml-cpp.cmake)
include(cmake/modules/uvgRTP.cmake)

message(STATUS "[MXRE COMPILE INFO]")

message("\t MXRE_CXX_FLAGS: ${MXRE_CXX_FLAGS}")
message("\t MXRE_LINKER_FLAGS: ${MXRE_LINKER_FLAGS}")
message("\t MXRE_LINKER_LIBS: ${MXRE_LINKER_LIBS}")
message("\t MXRE_SRCS: ${MXRE_SRCS}")

add_subdirectory(third-party)
add_subdirectory(src)
add_subdirectory(mxreinc)
add_subdirectory(examples)

# Install
string(REPLACE ";" " " MXRE_CXX_FLAGS "${MXRE_CXX_FLAGS}")
string(REPLACE ";" " " MXRE_LINKER_FLAGS "${MXRE_LINKER_FLAGS}")
string(REPLACE ";" " " MXRE_LINKER_LIBS "${MXRE_LINKER_LIBS}")

set(MXRE_HEADER mxre)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${MXRE_HEADER} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

if(NOT WIN32)
  mark_as_advanced( PKG_CONFIG_PATHWAY )
  set( PKG_CONFIG_PATH "/usr/local/lib/pkgconfig")

  string(REPLACE ":" ";" PREFIX_LIST "${CMAKE_SYSTEM_PREFIX_PATH}")
  list(FIND PREFIX_LIST ${CMAKE_INSTALL_PREFIX}  INSTALL_PREFIX_INDEX)

  #if(${INSTALL_PREFIX_INDEX} GREATER 0)
  #  file( MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/pkgconfig )
  #  set( PKG_CONFIG_PATH ${CMAKE_INSTALL_PREFIX}/pkgconfig )
  #endif()

  configure_file("mxre.pc.in" "mxre.pc" @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mxre.pc DESTINATION ${PKG_CONFIG_PATH}/)
endif(NOT WIN32)

