add_library(flexr_rtp_plugin SHARED flexr_rtp_plugin.cc)
set_target_properties(flexr_rtp_plugin PROPERTIES PREFIX "")
set_target_properties(flexr_rtp_plugin PROPERTIES OUTPUT_NAME "FlexrRtpPlugin")

target_include_directories(flexr_rtp_plugin PUBLIC "${PROJECT_SOURCE_DIR}")
target_link_libraries(flexr_rtp_plugin flexr_core)

