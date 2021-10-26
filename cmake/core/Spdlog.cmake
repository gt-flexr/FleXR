macro(core_use_spdlog)
  pkg_search_module(SPDLOG REQUIRED spdlog)
  if(SPDLOG_FOUND)
    message(STATUS "SPDLOG Details")
    message("\t SPDLOG_INCLUDE_DIRS: ${SPDLOG_INCLUDE_DIRS}")
    message("\t SPDLOG_LDFLAGS: ${SPDLOG_LDFLAGS}")

    include_directories(${SPDLOG_INCLUDE_DIRS})
    list(APPEND FLEXR_CORE_CXX_FLAGS ${SPDLOG_CFLAGS_OTHER})
    list(APPEND FLEXR_CORE_LINKER_FLAGS ${SPDLOG_LDFLAGS})
  endif(SPDLOG_FOUND)
endmacro()

