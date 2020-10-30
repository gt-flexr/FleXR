#ifndef __MXRE__
#define __MXRE__

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

/* defs */
#include "defs.h"

/* Types */
#include "types/clock_types.h"
#include "types/complex_yolo_types.h"

#include "types/cv/types.h"
#include "types/cv/orb_marker_tracker.h"

#include "types/gl/types.h"
#include "types/gl/mesh.h"
#include "types/gl/model.h"
#include "types/gl/camera.h"
#include "types/gl/shader.h"

#include "types/ar/virtual_world.h"
#include "types/ar/virtual_object.h"
#include "types/ar/virtual_world_manager.h"

#include "types/plugin/illixr_source.h"
#include "types/plugin/illixr_sink.h"

/* Utils */
#include "utils/getch.h"
#include "utils/path_finder.h"
#include "utils/cv_utils.h"
#include "utils/gl_utils.h"
#include "utils/egl_utils.h"
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

/* Kernels */
#include "kernels/sources/cv_camera.h"
#include "kernels/sources/keyboard.h"
#include "kernels/sources/image_loader.h"
#include "kernels/sources/appsource.h"

#include "kernels/sinks/cv_display.h"
#include "kernels/sinks/appsink.h"

#include "kernels/perceptions/prediction_receiver.h"
#include "kernels/perceptions/orb/orb_detector.h"
#include "kernels/perceptions/orb/cuda_orb_detector.h"
#include "kernels/perceptions/orb/object_ctx_extractor.h"

#include "kernels/offloading/message_sender.h"
#include "kernels/offloading/message_receiver.h"
#include "kernels/offloading/cvmat_sender.h"
#include "kernels/offloading/cvmat_receiver.h"
#include "kernels/offloading/static_sender.h"
#include "kernels/offloading/static_receiver.h"
#include "kernels/offloading/rtp_frame_sender.h"
#include "kernels/offloading/rtp_frame_receiver.h"

#include "kernels/app/object_renderer.h"
#include "kernels/app/complexyolo_renderer.h"

#endif

