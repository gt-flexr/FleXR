macro(core_use_catch2)
  pkg_search_module(CATCH2 REQUIRED catch2)
  if(CATCH2_FOUND)
    message(STATUS "CATCH2 Details")
    message("\t CATCH2_INCLUDE_DIRS: ${CATCH2_INCLUDE_DIRS}")
    message("\t CATCH2_LDFLAGS: ${CATCH2_LDFLAGS}")

    include_directories(${CATCH2_INCLUDE_DIRS})
    list(APPEND FLEXR_CORE_CXX_FLAGS ${CATCH2_CFLAGS_OTHER})
    list(APPEND FLEXR_CORE_LINKER_FLAGS ${CATCH2_LDFLAGS})
  endif(CATCH2_FOUND)
endmacro()

