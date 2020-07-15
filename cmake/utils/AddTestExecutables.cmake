
macro(add_test_executables test_srcs)
  foreach(test_src ${test_srcs})
    get_filename_component (NAME_WITHOUT_EXTENSION ${test_src} NAME_WE)
    set(test_out "${NAME_WITHOUT_EXTENSION}")

    add_executable(${test_out} ${test_src})
    target_compile_options(${test_out} PUBLIC ${MXRE_CXX_FLAGS})
    target_link_libraries(${test_out} ${MXRE_LINKER_FLAGS})
    target_link_libraries(${test_out} ${MXRE_LINKER_LIBS})
  endforeach()
endmacro()
