macro(util_flexr_option var desc val)
  set(${var} ${val})
endmacro()


macro(util_debug_mode value)
  if(${value})
    add_definitions(-D__DEBUG__=1)
    message(STATUS "Set Debug Mode... ON")
  else()
    add_definitions(-D__DEBUG__=0)
    message(STATUS "Set Debug Mode... OFF")
  endif()
endmacro()

