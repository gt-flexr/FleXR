#ifndef __FLEXR_COMP_YOLO_TYPE__
#define __FLEXR_COMP_YOLO_TYPE__

namespace flexr {
  namespace types {
      typedef struct ComplexYoloPrediction {
        float x;
        float y;
        float z;
        float ry;
        float cls_pred;
      } ComplexYoloPrediction;
  } // namespace types
} // namespace flexr

#endif

