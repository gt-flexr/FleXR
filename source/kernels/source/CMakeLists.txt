###############################################################################
# flexr_source_kernel
###############################################################################

#file(GLOB_RECURSE FLEXR_SOURCE_KERNEL_SRCS *.cc) # NOTE: Avoid GLOB in modern CMake

set(FLEXR_SOURCE_KERNEL_SRCS
  bag_camera.cc
  cv_camera.cc
  keyboard.cc
  rtp_frame_receiver.cc
)

add_library(flexr_source_kernel ${FLEXR_SOURCE_KERNEL_SRCS})
target_include_directories(flexr_source_kernel PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/flexrinc")

target_compile_options(flexr_source_kernel PUBLIC  ${FLEXR_CXX_FLAGS})
target_link_libraries(flexr_source_kernel  PRIVATE ${FLEXR_LINKER_FLAGS})
target_link_libraries(flexr_source_kernel  PRIVATE ${FLEXR_LINKER_LIBS})
target_link_libraries(flexr_source_kernel  PRIVATE flexr_component)

###############################################################################
# vulkan_renderer
###############################################################################

add_library(vulkan_renderer)

target_compile_features(vulkan_renderer PRIVATE cxx_std_17)

target_compile_options(vulkan_renderer PRIVATE ${FLEXR_CXX_FLAGS})

target_sources(vulkan_renderer
  PRIVATE
    renderer.cc
    vulkan_renderer.cc)

target_link_libraries(vulkan_renderer
  PUBLIC # NOTE: Needed as these are pulbicly included in renderer.h
    #Vulkan-Headers # TODO: Causes linker issues with non-vulkan examples
    vma
    renderdoc_app
  PRIVATE
    dl
    vk-bootstrap
    ${FLEXR_LINKER_FLAGS}
    ${FLEXR_LINKER_LIBS}
    flexr_component)

###############################################################################
# install
###############################################################################

install(TARGETS flexr_source_kernel vulkan_renderer DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

