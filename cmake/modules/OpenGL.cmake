# https://cmake.org/cmake/help/v3.10/module/FindOpenGL.html
macro(use_opengl)
  include(FindOpenGL)
  find_package(GLUT REQUIRED)

  message(STATUS "[OpenGL Details]")
  if (OPENGL_FOUND)
    message("\t OPENGL_INCLUDE_DIR: ${OPENGL_INCLUDE_DIR}")
    message("\t OPENGL_LIBRARY: ${OPENGL_LIBRARY}")
    message("\t OPENGL_EGL_INCLUDE_DIR: ${OPENGL_EGL_INCLUDE_DIR}")
    message("\t OPENGL_egl_LIBRARY: ${OPENGL_egl_LIBRARY}")

    include_directories(${OPENGL_INCLUDE_DIR})
    include_directories(${OPENGL_EGL_INCLUDE_DIR})
    list(APPEND MXRE_LINKER_LIBS ${OPENGL_LIBRARY} ${OPENGL_egl_LIBRARY})
  endif ()

  if(GLUT_FOUND)
    message("\t GLUT_LIBRARIES: ${GLUT_LIBRARIES}")
    list(APPEND MXRE_LINKER_LIBS ${GLUT_LIBRARIES})
  endif()

endmacro()
