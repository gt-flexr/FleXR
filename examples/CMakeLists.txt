file(GLOB_RECURSE EXAMPLE_SRCS
     runner.cc
     tutorials/*.cc
     tests/*.cc
     )

list(APPEND EXAMPLE_SRCS

)


foreach(EXAMPLE_SRC ${EXAMPLE_SRCS})
  get_filename_component (NAME_WITHOUT_EXTENSION ${EXAMPLE_SRC} NAME_WE)
  set(EXAMPLE_NAME "${NAME_WITHOUT_EXTENSION}")

  add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
  target_link_libraries(${EXAMPLE_NAME} flexr_kernel)
  target_link_libraries(${EXAMPLE_NAME} flexr_kernel_yaml)
endforeach(EXAMPLE_SRC ${EXAMPLE_SRCS})

