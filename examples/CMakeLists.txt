###############################################################################
# examples
###############################################################################

#file(GLOB_RECURSE EXAMPLE_SRCS runner.cc tutorials/*.cc tests/*.cc) # NOTE: Avoid GLOB in modern CMake

set(EXAMPLE_SRCS
  tutorials/old/bag_example.cc
  tutorials/old/cv_cam_example.cc
  tutorials/old/roundtrip_client.cc
  tutorials/old/roundtrip_server.cc
  tutorials/old/streaming_receiver.cc
  tutorials/old/streaming_sender.cc
)

list(APPEND EXAMPLE_SRCS
  marker_ar/old/orb_serialized.cc
  marker_ar/old/orb_local.cc
  marker_ar/old/orb_full_client.cc
  marker_ar/old/orb_part_detector_client.cc
  marker_ar/old/orb_part_perception_client.cc
  marker_ar/old/renderer_client.cc
  marker_ar/old/renderer_server.cc
  marker_ar/old/ue4_local.cc
)

if(${USE_OPENCV_CUDA})
list(APPEND EXAMPLE_SRCS
  marker_ar/old/orb_serialized_cuda.cc
  marker_ar/old/orb_local_cuda.cc
  marker_ar/old/orb_full_server.cc
  marker_ar/old/orb_part_detector_server.cc
  marker_ar/old/orb_part_perception_server.cc
)
endif(${USE_OPENCV_CUDA})

foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
  get_filename_component (NAME_WITHOUT_EXTENSION ${EXAMPLE_SRC} NAME_WE)
  set(EXAMPLE_NAME "${NAME_WITHOUT_EXTENSION}")

  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
  target_link_libraries(${EXAMPLE_NAME} flexr_kernel)
  target_link_libraries(${EXAMPLE_NAME} flexr_yaml_kernel)



  # TODO: Remove this
  # NOTE: Needed since all examples include <flexr> => <vulkan_renderer.h> => <renderer.h>
  # which includes these 2 library headers and C++17 features
  # TODO: <flexr> header needs to be organized better
  target_link_libraries(${EXAMPLE_NAME} vma renderdoc_app)
  target_compile_features(${EXAMPLE_NAME} PRIVATE cxx_std_17)
endforeach(EXAMPLE_SRC ${EXAMPLE_SRCS})

###############################################################################
# vulkan_renderer_example
###############################################################################

add_executable(vulkan_renderer_example)

target_compile_features(vulkan_renderer_example PRIVATE cxx_std_17)

target_sources(vulkan_renderer_example PRIVATE tutorials/old/vulkan_renderer_example.cc)

target_link_libraries(vulkan_renderer_example
  PRIVATE
    vulkan_renderer
    image_sink
    flexr_kernel)
