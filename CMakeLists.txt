cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
project(FLEXR CXX)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE Debug)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(PkgConfig REQUIRED)

include(cmake/utils/util.cmake)
include(cmake/core/core.cmake)
include(cmake/kernels/kernels.cmake)

util_print_os()
util_debug_mode(ON)

# Build core
util_flexr_option(USE_FFMPEG_NVCODEC "Use nvcodec with ffmpeg" OFF)
core_setup_dependencies()
add_subdirectory(flexr_third_party)
add_subdirectory(flexr_core)

# Enable kernels
# Source
util_flexr_option(KERNEL_BAG_CAMERA        "Use BagCamera Source kernel" OFF)
util_flexr_option(KERNEL_CV_CAMERA         "Use CVCamera Source kernel"  OFF)
util_flexr_option(KERNEL_KEYBOARD          "Use Keyboard Source kernel"  OFF)
# Sink
util_flexr_option(KERNEL_CV_DISPLAY        "Use CVDisplay Sink kernel"   OFF)
util_flexr_option(KERNEL_NON_DISPLAY       "Use NonDisplay Sink kernel"  OFF)
# Intermediate
util_flexr_option(KERNEL_ARUCO_CAM_LOCATOR      "Use ArUcoCamLocator Intermediate kernel"      OFF)
util_flexr_option(KERNEL_ARUCO_DETECTOR         "Use ArUcoDetector Intermediate kernel"        OFF)
util_flexr_option(KERNEL_ORB_CAM_LOCATOR        "Use OrbCamLocator Intermediate kernel"        OFF)
util_flexr_option(KERNEL_CUDA_ORB_CAM_LOCATOR   "Use CudaOrbCamLocator Intermediate kernel"    OFF)
util_flexr_option(KERNEL_SAMPLE_MARKER_RENDERER "Use SampleMarkerRenderer Intermediate kernel" OFF)
util_flexr_option(KERNEL_FRAME_DECODER          "Use FrameDecoder Intermediate kernel"         OFF)
util_flexr_option(KERNEL_FRAME_ENCODER          "Use FrameEncoder Intermediate kernel"         OFF)
util_flexr_option(KERNEL_FRAME_CONVERTER        "Use FrameConverter Intermediate kernel"       OFF)

# Build kernels
kernel_setup_dependencies()
add_subdirectory(flexr_kernels)

# Build tests & exmamples
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
add_subdirectory(tests)
add_subdirectory(examples)

# Installation settings
util_set_installation()

