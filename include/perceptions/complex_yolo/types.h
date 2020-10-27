#ifndef __MXRE_COMP_YOLO_TYPE__
#define __MXRE_COMP_YOLO_TYPE__

namespace mxre {
  namespace perception {
    namespace complex_yolo {

      typedef struct Prediction {
        float x;
        float y;
        float z;
        float ry;
        float cls_pred;
      } Prediction;

    } // namespace complex_yolo
  } // namespace perception
} // namespace mxre

#endif

