cmake_minimum_required(VERSION 3.21)
include(FetchContent)

project(vulkan_debug)

set(ROOT_DIR "..")

include(../shaders/CMakeLists.txt)

include(${ROOT_DIR}/cmake/modules/Vulkan.cmake)

add_executable(vulkan_debug)

add_dependencies(vulkan_debug shaders)

target_compile_features(vulkan_debug PRIVATE cxx_std_17)

target_include_directories(vulkan_debug PRIVATE ${ROOT_DIR}/flexrinc)

target_sources(vulkan_debug
  PRIVATE
    ${ROOT_DIR}/source/kernels/source/renderer.cc
    vulkan_debug.cc)

target_link_libraries(vulkan_debug
  PRIVATE
    dl
    Vulkan-Headers
    vk-bootstrap
    vma
    renderdoc_app
    stb_image_write)
