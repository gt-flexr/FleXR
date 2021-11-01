cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(FLEXR CXX)
#set(CMAKE_CXX_COMPILER g++) # This interferes with Vulkan.cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)

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

flexr_option(USE_FFMPEG_NVCODEC "Use nvcodec with ffmpeg" OFF)
flexr_option(USE_OPENCV_CUDA "Use cuda-accelerated functions of OpenCV" OFF)

# Source codes & headers
configure_file(config/root_directory.h.in config/root_directory.h)

# FLEXR compile info
set(FLEXR_CXX_FLAGS "")
set(FLEXR_LINKER_FLAGS "")
set(FLEXR_LINKER_LIBS "")

# Include cmake modules
include_directories("flexrinc")
include(cmake/modules/OpenGL.cmake)
include(cmake/modules/OpenCV.cmake)
include(cmake/modules/RaftLib.cmake)
include(cmake/modules/LibZMQ.cmake)
include(cmake/modules/FFmpeg.cmake)
include(cmake/modules/Spdlog.cmake)
include(cmake/modules/yaml-cpp.cmake)
include(cmake/modules/uvgRTP.cmake)
include(cmake/modules/rosbag.cmake)
include(cmake/modules/renderer.cmake)

message(STATUS "[FLEXR COMPILE INFO]")

message("\t FLEXR_CXX_FLAGS: ${FLEXR_CXX_FLAGS}")
message("\t FLEXR_LINKER_FLAGS: ${FLEXR_LINKER_FLAGS}")
message("\t FLEXR_LINKER_LIBS: ${FLEXR_LINKER_LIBS}")

add_subdirectory(third-party)
add_subdirectory(flexrinc)
add_subdirectory(source)
add_subdirectory(examples)
add_subdirectory(shaders)

# Install
string(REPLACE ";" " " FLEXR_CXX_FLAGS "${FLEXR_CXX_FLAGS}")
string(REPLACE ";" " " FLEXR_LINKER_FLAGS "${FLEXR_LINKER_FLAGS}")
string(REPLACE ";" " " FLEXR_LINKER_LIBS "${FLEXR_LINKER_LIBS}")

set(FLEXR_HEADER flexr)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${FLEXR_HEADER} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

if(NOT WIN32)
  if(NOT DEFINED ENV{PKG_CONFIG_PATH})
    set( PKG_CONFIG_PATH "/usr/local/lib/pkgconfig")
    message("PKG_CONFIG_PATH is not found. Set it with ${PKG_CONFIG_PATH}")
  endif(NOT DEFINED ENV{PKG_CONFIG_PATH})

  configure_file("flexr.pc.in" "flexr.pc" @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/flexr.pc DESTINATION ${PKG_CONFIG_PATH}/)
endif(NOT WIN32)

