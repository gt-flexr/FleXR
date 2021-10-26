file(GLOB_RECURSE FLEXR_CORE_COMPONENT_SRCS *.cc)

add_library(flexr_core_component ${FLEXR_CORE_COMPONENT_SRCS})
target_include_directories(flexr_core_component PUBLIC "${PROJECT_SOURCE_DIR}")

target_compile_options(flexr_core_component PUBLIC ${FLEXR_CORE_CXX_FLAGS})
target_link_libraries(flexr_core_component ${FLEXR_CORE_LINKER_FLAGS})
target_link_libraries(flexr_core_component ${FLEXR_CORE_LINKER_LIBS})

set(FLEXR_CORE_COMPONENT flexr_core_component)

install(TARGETS flexr_core_component ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

