#ifndef __MXRE_DEFS__
#define __MXRE_DEFS__

#define MAX_OBJS 5
#define TOTAL_FRAMES 1000

// GL scale factor
#define SCALE_FACTOR 10

// Index for attributes of cvMat
#define MXRE_DTYPE_PRIMITIVE 0
#define MXRE_DTYPE_FRAME 1

#define MX_MAT_ATTR_NUM 4
#define MX_MAT_SIZE_IDX 0
#define MX_MAT_ROWS_IDX 1
#define MX_MAT_COLS_IDX 2
#define MX_MAT_TYPE_IDX 3

// SDP buf size
#define SDP_BUF_SIZE 2048

// RTP payload size for uvgRTP
#define MXRE_RTP_PAYLOAD_UNIT_SIZE 1443
#define MXRE_RTP_TRACKING_INVOICE (-777777)

// color-print reference: http://web.theurbanpenguin.com/adding-color-to-your-output-from-c/
#define debug_print(...) do { if (__DEBUG__) { \
                              fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                              fprintf(stderr, __VA_ARGS__); \
                              fprintf(stderr, "\n"); } \
                            } while (0)

#ifdef __PROFILE__
#define profile_print(...) do { \
                            fprintf(stderr, "\033[1;34m[PROFILE] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
                            fprintf(stderr, __VA_ARGS__); \
                            fprintf(stderr, "\n");  \
                           } while (0)
#endif

#include <chrono>
#define getExeTime(a,b) ( (double)(std::chrono::duration_cast<std::chrono::microseconds>(a-b).count()) / 1000 )
#define getTimeStampOf(a) ( (double)std::chrono::duration_cast<std::chrono::microseconds>(a.time_since_epoch()).count() / 1000 )
#define getNow() std::chrono::high_resolution_clock::now()
#define getTimeStampNow() ( (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / 1000 )
#define getTimeStampNowUint() (unsigned int)( (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() / 1000 )
#define convertTimeStampDouble2Uint(a) (uint32_t)(a)

#endif

