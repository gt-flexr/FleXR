set(MXRE_THIRD_PARTIES "")
set( MXRE_INSTALL_INC ${CMAKE_INSTALL_PREFIX}/include/mxreinc )

add_library(STB_IMAGE "stb/stb_image.cpp")
set(MXRE_THIRD_PARTIES ${MXRE_THIRD_PARTIES} STB_IMAGE)
message(STATUS "[MXRE_THIRD_PARTIES]")
message("\t${MXRE_THIRD_PARTIES}")

install( FILES stb/stb_image.h DESTINATION ${MXRE_INC_TARGET}/stb )

