#ifndef __MXRE__
#define __MXRE__

/* defs */
#include "defs.h"


/* utils */
#include "gl_utils.h"
#include "gl_types.h"
#include "egl_utils.h"
#include "cv_types.h"


/* cv_units */
#include "object_tracker.h"


/* pipeline */
/** input_srcs **/
#include "camera.h"

/** ctx_understanding **/
#include "object_detector.h"

/** contextualizing **/
#include "object_ctx_extractor.h"

/** rendering **/
#include "object_renderer.h"

/** network **/
#include "network/message_sender.h"
#include "network/message_receiver.h"

#include "network/static_sender.h"
#include "network/static_receiver.h"

#include "network/cvmat_receiver.h"
#include "network/cvmat_sender.h"

#include "network/rtp_frame_sender.h"
#include "network/rtp_frame_receiver.h"

#endif
