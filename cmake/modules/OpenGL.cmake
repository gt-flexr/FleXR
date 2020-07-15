macro(use_opengl)
  find_package(OpenGL REQUIRED)
  find_package(GLUT REQUIRED)

  if(OPENGL_FOUND AND GLUT_FOUND)
    message(STATUS "[OpenGL Details]")
    message(STATUS "\t OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
    message(STATUS "\t GLUT_LIBRARIES: ${GLUT_LIBRARIES}")

    list(APPEND MXRE_LINKER_LIBS ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
  endif(OPENGL_FOUND AND GLUT_FOUND)
endmacro()
