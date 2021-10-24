###############################################################################
# flexr_sink_kernel
###############################################################################

#file(GLOB_RECURSE FLEXR_SINK_KERNEL_SRCS *.cc) # NOTE: Avoid GLOB in modern CMake

set(FLEXR_SINK_KERNEL_SRCS
  cv_display.cc
  non_display.cc
  rtp_frame_sender.cc
)

add_library(flexr_sink_kernel ${FLEXR_SINK_KERNEL_SRCS})
target_include_directories(flexr_sink_kernel PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           "${PROJECT_SOURCE_DIR}/flexrinc")

target_compile_options(flexr_sink_kernel PUBLIC ${FLEXR_CXX_FLAGS})
target_link_libraries(flexr_sink_kernel ${FLEXR_LINKER_FLAGS})
target_link_libraries(flexr_sink_kernel ${FLEXR_LINKER_LIBS})
target_link_libraries(flexr_sink_kernel flexr_type)
target_link_libraries(flexr_sink_kernel flexr_component)

###############################################################################
# image_sink
###############################################################################

add_library(image_sink)

target_compile_features(image_sink PRIVATE cxx_std_17)

target_compile_options(image_sink PRIVATE ${FLEXR_CXX_FLAGS})

target_sources(image_sink PRIVATE image_sink.cc)

target_link_libraries(image_sink
  PRIVATE
    stb_image_write
    ${FLEXR_LINKER_FLAGS}
    ${FLEXR_LINKER_LIBS}
    flexr_component)

###############################################################################
# install
###############################################################################

install(TARGETS flexr_sink_kernel image_sink  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

