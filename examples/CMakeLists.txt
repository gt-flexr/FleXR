file(GLOB_RECURSE EXAMPLE_SRCS modules/*.cc)
list(APPEND EXAMPLE_SRCS
  basic_example.cc
  fulloff_client.cc
  fulloff_server.cc
)

if(${USE_OPENCV_CUDA})
list(APPEND EXAMPLE_SRCS
  basic_example_with_cuda.cc
  fulloff_server_with_cuda.cc
)
endif(${USE_OPENCV_CUDA})

foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
  get_filename_component (NAME_WITHOUT_EXTENSION ${EXAMPLE_SRC} NAME_WE)
  set(EXAMPLE_NAME "${NAME_WITHOUT_EXTENSION}")

  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
  target_include_directories(${EXAMPLE_NAME} PUBLIC
                               "${PROJECT_SOURCE_DIR}/include"
                               "${CMAKE_BINARY_DIR}/config")

  target_compile_options(${EXAMPLE_NAME} PUBLIC ${MXRE_CXX_FLAGS})
  target_link_libraries(${EXAMPLE_NAME} ${MXRE_LINKER_FLAGS})
  target_link_libraries(${EXAMPLE_NAME} ${MXRE_LINKER_LIBS})
  target_link_libraries(${EXAMPLE_NAME} ${MXRE_THIRD_PARTIES})
  target_link_libraries(${EXAMPLE_NAME} mxre)
endforeach(EXAMPLE_SRC ${EXAMPLE_SRCS})

