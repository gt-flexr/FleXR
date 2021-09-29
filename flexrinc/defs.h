#ifndef __FLEXR_DEFS__
#define __FLEXR_DEFS__

/// Mathematical constant PI
#define PI (3.141592)

/// Max rendering objects for flexr::kernels::ObjectRenderer
#define MAX_OBJS 5
/// Rendering scale factor for depth calibration of flexr::kernels::ObjectRenderer
#define SCALE_FACTOR 10

/// Max detecting marakers for flexr::kernels::ORBDetector
#define MAX_MARKERS 10

/// Tag size of FleXR messages
#define FLEXR_MSG_TAG_SIZE 32

/// Debug print function
#define debug_print(...) do { \
                              fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                              fprintf(stderr, __VA_ARGS__); \
                              fprintf(stderr, "\n"); \
                            } while (0)

/// Profiling print function
#ifdef __PROFILE__
#define profile_print(...) do { \
                            fprintf(stderr, "\033[1;34m[PROFILE] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                            fprintf(stderr, __VA_ARGS__); \
                            fprintf(stderr, "\n");  \
                           } while (0)
#endif

#include <chrono>

/// Macro function getting high resolution clock
#define getNow() std::chrono::high_resolution_clock::now()

/// Macro function getting the timestamp of high resolution clock in microsecond
#define getTsUs() (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count())

/// Macro function getting the timestamp of high resolution clock in millisecond
#define getTsMs() ( ((double)getTsUs()) / 1000)

/// Macro function getting the timestamp of high resolution clock in millisecond
#define getTsNow() getTsMs()

#endif

