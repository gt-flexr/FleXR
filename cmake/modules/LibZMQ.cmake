macro(use_libzmq)
  pkg_search_module(LIBZMQ REQUIRED libzmq)
  if(LIBZMQ_FOUND)
    message(STATUS "[LibZMQ Details]")
    message("\t LIBZMQ_INCLUDE_DIRS: ${LIBZMQ_INCLUDE_DIRS}")
    message("\t LIBZMQ_LDFLAGS: ${LIBZMQ_LDFLAGS}")

    include_directories(${LIBZMQ_INCLUDE_DIRS})
    list(APPEND MXRE_CXX_FLAGS ${LIBZMQ_CFLAGS_OTHER})
    list(APPEND MXRE_LINKER_FLAGS ${LIBZMQ_LDFLAGS})
  endif(LIBZMQ_FOUND)
endmacro()

use_libzmq()

