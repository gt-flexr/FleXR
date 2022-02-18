#ifndef __FLEXR_KERNEL__
#define __FLEXR_KERNEL__

#include "flexr_kernels/include/intermediate/aruco_cam_locator.h"
#include "flexr_kernels/include/intermediate/aruco_detector.h"
#include "flexr_kernels/include/intermediate/frame_converter.h"
#include "flexr_kernels/include/intermediate/frame_decoder.h"
#include "flexr_kernels/include/intermediate/frame_encoder.h"
#include "flexr_kernels/include/intermediate/nvmpi_decoder.h"
#include "flexr_kernels/include/intermediate/nvmpi_encoder.h"
#include "flexr_kernels/include/intermediate/orb_cam_locator.h"
#include "flexr_kernels/include/intermediate/cuda_orb_cam_locator.h"
#include "flexr_kernels/include/intermediate/sample_marker_renderer.h"

#include "flexr_kernels/include/sink/cv_display.h"
#include "flexr_kernels/include/sink/non_display.h"

#include "flexr_kernels/include/source/bag_camera.h"
#include "flexr_kernels/include/source/cv_camera.h"
#include "flexr_kernels/include/source/keyboard.h"


#include "flexr_kernels/include/yaml/yaml_parser.h"

#include "flexr_kernels/include/yaml/intermediate/yaml_aruco_cam_locator.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_aruco_detector.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_frame_converter.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_frame_decoder.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_frame_encoder.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_orb_cam_locator.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_cuda_orb_cam_locator.h"
#include "flexr_kernels/include/yaml/intermediate/yaml_sample_marker_renderer.h"

#include "flexr_kernels/include/yaml/sink/yaml_cv_display.h"
#include "flexr_kernels/include/yaml/sink/yaml_non_display.h"

#include "flexr_kernels/include/yaml/source/yaml_bag_camera.h"
#include "flexr_kernels/include/yaml/source/yaml_cv_camera.h"
#include "flexr_kernels/include/yaml/source/yaml_keyboard.h"

#endif

