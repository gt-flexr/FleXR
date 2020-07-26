file(GLOB_RECURSE TEST_SRCS *.cc)

foreach(TEST_SRC ${TEST_SRCS})
  get_filename_component (NAME_WITHOUT_EXTENSION ${TEST_SRC} NAME_WE)
  set(TEST_NAME "${NAME_WITHOUT_EXTENSION}")

  add_executable(${TEST_NAME} ${TEST_SRC})
  target_compile_options(${TEST_NAME} PUBLIC ${MXRE_CXX_FLAGS})
  target_link_libraries(${TEST_NAME} ${MXRE_LINKER_FLAGS})
  target_link_libraries(${TEST_NAME} ${MXRE_LINKER_LIBS})
  target_link_libraries(${TEST_NAME} mxre)
endforeach(TEST_SRC ${TEST_SRCS})

