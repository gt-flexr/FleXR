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

  incubator/unity_test.cc
  incubator/unity_writer.cc
)


if(${USE_OPENCV_CUDA})
list(APPEND EXAMPLE_SRCS
  incubator/illixr_local_server.cc
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
  target_include_directories(${EXAMPLE_NAME} PUBLIC
                             "${PROJECT_SOURCE_DIR}/mxreinc"
                             "${CMAKE_BINARY_DIR}/config")

  target_compile_options(${EXAMPLE_NAME} PUBLIC ${MXRE_CXX_FLAGS})
  target_link_libraries(${EXAMPLE_NAME} ${MXRE_LINKER_FLAGS})
  target_link_libraries(${EXAMPLE_NAME} ${MXRE_LINKER_LIBS})
  target_link_libraries(${EXAMPLE_NAME} ${MXRE_THIRD_PARTIES})
  target_link_libraries(${EXAMPLE_NAME} mxre)
endforeach(EXAMPLE_SRC ${EXAMPLE_SRCS})

