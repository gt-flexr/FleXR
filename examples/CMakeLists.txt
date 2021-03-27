file(GLOB_RECURSE EXAMPLE_SRCS modules/*.cc)
list(APPEND EXAMPLE_SRCS
  ar_with_lidar/complex_yolo.cc

  #marker_ar_with_cam/full_offloading/cam_fulloff_client.cc
  #marker_ar_with_cam/full_offloading/cam_fulloff_server.cc
  #marker_ar_with_cam/basic_example.cc

  marker_ar_with_mock/mock_cam_q.cc
  marker_ar_with_mock/mock_cam_adq.cc
  marker_ar_with_mock/serialized_mock_cam.cc
  marker_ar_with_mock/full_offloading/mock_client.cc

  #marker_ar_with_img/full_offloading/img_fulloff_server.cc
  #marker_ar_with_img/full_offloading/img_fulloff_client.cc

  incubator/unity_test.cc
  incubator/unity_writer.cc
)


if(${USE_OPENCV_CUDA})
list(APPEND EXAMPLE_SRCS
  #marker_ar_with_cam/full_offloading/cam_fulloff_server_with_cuda.cc
  #marker_ar_with_cam/basic_example_with_cuda.cc

  marker_ar_with_mock/mock_cam_cuda_q.cc
  marker_ar_with_mock/mock_cam_cuda_adq.cc
  marker_ar_with_mock/serialized_mock_cam_cuda.cc
  marker_ar_with_mock/full_offloading/mock_server.cc

  #incubator/illixr_local_server.cc
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

