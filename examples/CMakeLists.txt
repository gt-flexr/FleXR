file(GLOB_RECURSE EXAMPLE_SRCS modules/*.cc)
list(APPEND EXAMPLE_SRCS
  basic_example.cc
  complex_yolo.cc

  full_offloading/fulloff_client.cc
  full_offloading/fulloff_server.cc
  partial_offloading/detector_only_client.cc
  partial_offloading/detector_only_server.cc
  partial_offloading/perception_only_client.cc
  partial_offloading/perception_only_server.cc
)

if(${USE_OPENCV_CUDA})
list(APPEND EXAMPLE_SRCS
  basic_example_with_cuda.cc
  imgloader_example_with_cuda.cc
  illixr_local_server.cc
  full_offloading/fulloff_server_with_cuda.cc
  partial_offloading/detector_only_server_with_cuda.cc
  partial_offloading/perception_only_server_with_cuda.cc
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

