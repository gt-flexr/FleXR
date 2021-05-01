set(MXRE_THIRD_PARTIES "")
set( MXRE_INSTALL_INC ${CMAKE_INSTALL_PREFIX}/include/mxreinc )

add_library(STB_IMAGE "stb/stb_image.cpp")
set(MXRE_THIRD_PARTIES ${MXRE_THIRD_PARTIES} STB_IMAGE)
message(STATUS "[MXRE_THIRD_PARTIES]")
message("\t${MXRE_THIRD_PARTIES}")

install( FILES stb/stb_image.h DESTINATION ${MXRE_INC_TARGET}/stb )

# vertex, fragment, geometric shaders
file(GLOB_RECURSE MXRE_SHADERS *.vs *.fs *.gs)
foreach(MXRE_SHADER ${MXRE_SHADERS})
  file(COPY ${MXRE_SHADER} DESTINATION ${CMAKE_SOURCE_DIR}/build/examples)
endforeach(MXRE_SHADER)

