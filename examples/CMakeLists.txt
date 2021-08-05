file(GLOB_RECURSE EXAMPLE_SRCS
     tutorials/*.cc
     tests/*.cc
     )

list(APPEND EXAMPLE_SRCS
  marker_ar/local/orb_serialized.cc
  marker_ar/local/orb_local.cc
  marker_ar/offloading/orb_full_client.cc
  marker_ar/offloading/orb_part_detector_client.cc
  marker_ar/offloading/orb_part_perception_client.cc
  marker_ar/offloading/renderer_client.cc
  marker_ar/offloading/renderer_server.cc
  marker_ar/aruco/ue4_local.cc
)


if(${USE_OPENCV_CUDA})
list(APPEND EXAMPLE_SRCS
  marker_ar/local/orb_serialized_cuda.cc
  marker_ar/local/orb_local_cuda.cc
  marker_ar/offloading/orb_full_server.cc
  marker_ar/offloading/orb_part_detector_server.cc
  marker_ar/offloading/orb_part_perception_server.cc
)
endif(${USE_OPENCV_CUDA})


foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
  get_filename_component (NAME_WITHOUT_EXTENSION ${EXAMPLE_SRC} NAME_WE)
  set(EXAMPLE_NAME "${NAME_WITHOUT_EXTENSION}")

  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
  target_link_libraries(${EXAMPLE_NAME} flexr_kernel)
endforeach(EXAMPLE_SRC ${EXAMPLE_SRCS})

