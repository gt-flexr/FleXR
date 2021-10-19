add_library(flexr_core INTERFACE)

add_subdirectory(components)

target_link_libraries(flexr_core INTERFACE flexr_core_component)

