macro(core_use_opencv)
  pkg_search_module(OPENCV opencv opencv4)
  if(OPENCV_FOUND)
    message(STATUS "OpenCV Details")
    message("\t OPENCV_INCLUDE_DIRS: ${OPENCV_INCLUDE_DIRS}")
    message("\t OPENCV_LDFLAGS: ${OPENCV_LDFLAGS}")

    include_directories(${OPENCV_INCLUDE_DIRS})
    list(APPEND FLEXR_CORE_CXX_FLAGS ${OPENCV_CFLAGS_OTHER})
    list(APPEND FLEXR_CORE_LINKER_FLAGS ${OPENCV_LDFLAGS})
  endif(OPENCV_FOUND)
endmacro()

