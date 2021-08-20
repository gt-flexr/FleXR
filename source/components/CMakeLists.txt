file(GLOB_RECURSE FLEXR_COMPONENT_SRCS *.cc)

add_library(flexr_component ${FLEXR_COMPONENT_SRCS})
target_include_directories(flexr_component PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/flexrinc"
                           "${PROJECT_SOURCE_DIR}/third-party"
                           "${CMAKE_BINARY_DIR}/config")

target_compile_options(flexr_component PUBLIC ${FLEXR_CXX_FLAGS})
target_link_libraries(flexr_component ${FLEXR_LINKER_FLAGS})
target_link_libraries(flexr_component ${FLEXR_LINKER_LIBS})

set(FLEXR_COMPONENT flexr_component)

install(TARGETS flexr_component ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

