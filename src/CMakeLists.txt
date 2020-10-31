file(GLOB_RECURSE MXRE_SRCS *.cc *.vs *.fs *.gs)
file(GLOB_RECURSE MXRE_SHADERS *.vs *.fs *.gs)

add_library(mxre ${MXRE_SRCS})
target_include_directories(mxre PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/mxreinc"
                           "${PROJECT_SOURCE_DIR}/third-party"
                           "${CMAKE_BINARY_DIR}/config")


#target_compile_options(mxre PUBLIC "-W" "-Wall" "-Wextra" "-Wpedantic")
target_compile_options(mxre PUBLIC ${MXRE_CXX_FLAGS})
target_link_libraries(mxre ${MXRE_THIRD_PARTIES})
target_link_libraries(mxre ${MXRE_LINKER_FLAGS})
target_link_libraries(mxre ${MXRE_LINKER_LIBS})

# vertex, fragment, geometric shaders
foreach(MXRE_SHADER ${MXRE_SHADERS})
  file(COPY ${MXRE_SHADER} DESTINATION ${CMAKE_SOURCE_DIR}/build/examples)
endforeach(MXRE_SHADER)

install(TARGETS mxre ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

