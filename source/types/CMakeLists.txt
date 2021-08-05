file(GLOB_RECURSE FLEXR_TYPE_SRCS *.cc)

add_library(flexr_type ${FLEXR_TYPE_SRCS})
target_include_directories(flexr_type PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/flexrinc"
                           "${PROJECT_SOURCE_DIR}/third-party"
                           "${CMAKE_BINARY_DIR}/config")

target_compile_options(flexr_type PUBLIC ${FLEXR_CXX_FLAGS})
target_link_libraries(flexr_type ${FLEXR_LINKER_FLAGS})
target_link_libraries(flexr_type ${FLEXR_THIRD_PARTY})
target_link_libraries(flexr_type ${FLEXR_LINKER_LIBS})

install(TARGETS flexr_type ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

