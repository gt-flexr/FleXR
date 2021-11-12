cmake_minimum_required(VERSION 3.21)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/../shaders)

set(SHADERS
  ${SHADER_DIR}/forward_rendering.vert
  ${SHADER_DIR}/forward_rendering.frag)

# Create output directory for SPIRV shaders
set(OUT_DIR shaders)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${OUT_DIR})

# Compile each shader using glslc and add to SPV_SHADERS list
foreach(SHADER IN LISTS SHADERS)
  cmake_path(GET SHADER FILENAME  filename)
  cmake_path(GET SHADER STEM      stem)
  cmake_path(GET SHADER EXTENSION extension)
  add_custom_command(
    OUTPUT ${OUT_DIR}/${filename}.spv
    COMMAND glslc ${SHADER} -o ${OUT_DIR}/${filename}.spv
    DEPENDS ${SHADER}
    COMMENT "Compiling shader ${stem}${extension} into ${filename}.spv")
  list(APPEND SPV_SHADERS ${OUT_DIR}/${filename}.spv)
endforeach()

# Trigger shader compilation during build
add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
