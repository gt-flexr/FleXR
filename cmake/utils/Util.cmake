macro(mxre_option variable value)
  if(NOT DEFINED ${variable})
    set(${variable} ${value})
  endif()
endmacro()

macro(build_option variable value)
  # BUILD_TYPE: Release, Debug
  if(${variable} STREQUAL BUILD_TYPE)
    if(${value} STREQUAL Debug)
      add_definitions(-D__DEBUG__=1)
    else()
      add_definitions(-D__DEBUG__=0)
    endif()
  endif()

  # PROFILE: ON OFF
  if(${variable} STREQUAL PROFILE)
    if(${value})
      add_definitions(-D__PROFILE__=1)
    else()
      add_definitions(-D__PROFILE__=0)
    endif()
  endif()
endmacro()

