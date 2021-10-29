cmake_minimum_required(VERSION 3.21)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/../shaders)

set(SHADERS
  ${SHADER_DIR}/default.vert
  ${SHADER_DIR}/default.frag)

foreach(SHADER IN LISTS SHADERS)
  cmake_path(GET SHADER FILENAME filename)
  cmake_path(GET SHADER STEM stem)
  cmake_path(GET SHADER EXTENSION extension)
  add_custom_command(OUTPUT ${filename}.spv
    COMMAND glslc ${SHADER} -o ${filename}.spv
    DEPENDS ${SHADER}
    COMMENT "Compiling shader ${stem}${extension} into ${filename}.spv")
  list(APPEND SPV_SHADERS ${filename}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
