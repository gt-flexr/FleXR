set( FLEXR_THIRD_PARTY_INC_DIR "flexr_third_party/include" )

install( DIRECTORY ${CMAKE_SOURCE_DIR}/${FLEXR_THIRD_PARTY_INC_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

add_subdirectory(unity_plugins)

