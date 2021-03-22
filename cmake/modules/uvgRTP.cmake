macro(use_uvgrtp)
  pkg_search_module(UVGRTP REQUIRED uvgrtp)
  if(UVGRTP_FOUND)
    message(STATUS "[uvgRTP Details]")
    message("\t UVGRTP_INCLUDE_DIRS: ${UVGRTP_INCLUDE_DIRS}")
    message("\t UVGRTP_LDFLAGS: ${UVGRTP_LDFLAGS}")

    include_directories(${UVGRTP_INCLUDE_DIRS})
    list(APPEND MXRE_CXX_FLAGS ${UVGRTP_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${UVGRTP_LDFLAGS})
  endif(UVGRTP_FOUND)
endmacro()

use_uvgrtp()

