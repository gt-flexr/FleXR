file(GLOB_RECURSE MXRE_COMPONENT_SRCS *.cc)

add_library(mxre_component ${MXRE_COMPONENT_SRCS})
target_include_directories(mxre_component PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/mxreinc"
                           "${PROJECT_SOURCE_DIR}/third-party"
                           "${CMAKE_BINARY_DIR}/config")

target_compile_options(mxre_component PUBLIC ${MXRE_CXX_FLAGS})
target_link_libraries(mxre_component ${MXRE_LINKER_FLAGS})
target_link_libraries(mxre_component ${MXRE_LINKER_LIBS})

set(MXRE_COMPONENT mxre_component)

install(TARGETS mxre_component ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

