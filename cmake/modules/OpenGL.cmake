# https://cmake.org/cmake/help/v3.10/module/FindOpenGL.html

macro(use_opengl)
  pkg_search_module(OGL REQUIRED gl)
  if(OGL_FOUND)
    list(APPEND MXRE_CXX_FLAGS ${OGL_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${OGL_LDFLAGS})
  endif()


  pkg_search_module(EGL REQUIRED egl)
  if(EGL_FOUND)
    list(APPEND MXRE_CXX_FLAGS ${EGL_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${EGL_LDFLAGS})

  endif()


  pkg_search_module(GLEW REQUIRED glew)
  if(GLEW_FOUND)
    #message("\t GLEW_LIBRARIES: ${GLEW_LIBRARIES}")
    #list(APPEND MXRE_LINKER_LIBS ${GLEW_LIBRARIES})
    list(APPEND MXRE_CXX_FLAGS ${GLEW_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${GLEW_LDFLAGS})
  endif()


  pkg_search_module(GLM REQUIRED glm)
  if(GLM_FOUND)
    list(APPEND MXRE_CXX_FLAGS ${GLM_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${GLM_LDFLAGS})
  endif()


  pkg_search_module(ASSIMP REQUIRED assimp)
  if(ASSIMP_FOUND)
    list(APPEND MXRE_CXX_FLAGS ${ASSIMP_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${ASSIMP_LDFLAGS})
  endif()

endmacro()

use_opengl()

