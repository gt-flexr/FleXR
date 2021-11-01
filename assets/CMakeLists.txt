cmake_minimum_required(VERSION 3.21)

set(ASSET_DIR ${CMAKE_SOURCE_DIR}/../assets)

add_custom_target(copy-assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${ASSET_DIR}
  ${CMAKE_BINARY_DIR}/assets)
