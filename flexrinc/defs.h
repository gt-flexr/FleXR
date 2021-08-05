#ifndef __FLEXR_DEFS__
#define __FLEXR_DEFS__

#define PI (3.141592)

#define MAX_OBJS 5
#define MAX_MARKERS 10

// GL scale factor
#define SCALE_FACTOR 10

// Index for attributes of cvMat
#define FLEXR_DTYPE_PRIMITIVE 0
#define FLEXR_DTYPE_FRAME 1

// RTP payload size for uvgRTP
#define FLEXR_RTP_PAYLOAD_UNIT_SIZE 1443
#define FLEXR_RTP_TRACKING_INVOICE (-777777)

// For Message
#define FLEXR_MSG_TAG_SIZE 32

// color-print reference: http://web.theurbanpenguin.com/adding-color-to-your-output-from-c/
#define debug_print(...) do { \
                              fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                              fprintf(stderr, __VA_ARGS__); \
                              fprintf(stderr, "\n"); \
                            } while (0)

#ifdef __PROFILE__
#define profile_print(...) do { \
                            fprintf(stderr, "\033[1;34m[PROFILE] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                            fprintf(stderr, __VA_ARGS__); \
                            fprintf(stderr, "\n");  \
                           } while (0)
#endif

#include <chrono>
#define getNow() std::chrono::high_resolution_clock::now()
#define getTsNow() ( (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / 1000 )
#define getTsUs() (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())
#define getTsMs() ( ((double)getTsUs()) / 1000)

#endif

