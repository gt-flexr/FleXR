#ifndef __FLEXR_CORE_DEFS__
#define __FLEXR_CORE_DEFS__

/// Mathematical constant PI
#define PI (3.141592)

/// Tag size of FleXR messages
#define FLEXR_MSG_TAG_SIZE 32

/// Debug print function
#define debug_print(...) do { \
                              fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                              fprintf(stderr, __VA_ARGS__); \
                              fprintf(stderr, "\n"); \
                            } while (0)

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

