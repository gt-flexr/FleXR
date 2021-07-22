macro(use_opencv)
  pkg_search_module(OPENCV opencv opencv4)
  if(OPENCV_FOUND)
    message(STATUS "[OpenCV Details]")
    message("\t OPENCV_INCLUDE_DIRS: ${OPENCV_INCLUDE_DIRS}")
    message("\t OPENCV_LDFLAGS: ${OPENCV_LDFLAGS}")

    include_directories(${OPENCV_INCLUDE_DIRS})
    list(APPEND MXRE_CXX_FLAGS ${OPENCV_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${OPENCV_LDFLAGS})

    if(${USE_OPENCV_CUDA})
      add_definitions(-D__USE_OPENCV_CUDA__)
    endif(${USE_OPENCV_CUDA})
  endif(OPENCV_FOUND)
endmacro()

use_opencv()

