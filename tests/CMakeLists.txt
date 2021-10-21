file(GLOB_RECURSE TEST_SRCS
  *.cc
  )

find_package(Catch2 3 REQUIRED)

foreach(TEST_SRC ${TEST_SRCS})
  get_filename_component (NAME_WITHOUT_EXTENSION ${TEST_SRC} NAME_WE)
  set(TEST_NAME "TEST_${NAME_WITHOUT_EXTENSION}")

  add_executable(${TEST_NAME} ${TEST_SRC})
  target_link_libraries(${TEST_NAME} flexr_core)
  target_link_libraries(${TEST_NAME} flexr_kernel)
  target_link_libraries(${TEST_NAME} flexr_kernel_yaml)
  target_link_libraries(${TEST_NAME} Catch2::Catch2WithMain)
endforeach(TEST_SRC ${TEST_SRCS})

