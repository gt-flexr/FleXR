set(FLEXR_THIRD_PARTIES "")
set( FLEXR_INSTALL_INC ${CMAKE_INSTALL_PREFIX}/include/flexrinc )

add_library(STB_IMAGE "stb/stb_image.cpp")
set(FLEXR_THIRD_PARTIES ${FLEXR_THIRD_PARTIES} STB_IMAGE)
message(STATUS "[FLEXR_THIRD_PARTIES]")
message("\t${FLEXR_THIRD_PARTIES}")

install( FILES stb/stb_image.h DESTINATION ${FLEXR_INC_TARGET}/stb )

# vertex, fragment, geometric shaders
file(GLOB_RECURSE FLEXR_SHADERS *.vs *.fs *.gs)
foreach(FLEXR_SHADER ${FLEXR_SHADERS})
  file(COPY ${FLEXR_SHADER} DESTINATION ${CMAKE_SOURCE_DIR}/build/examples)
endforeach(FLEXR_SHADER)

