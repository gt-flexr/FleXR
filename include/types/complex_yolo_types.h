#ifndef __MXRE_COMP_YOLO_TYPE__
#define __MXRE_COMP_YOLO_TYPE__

namespace mxre {
  namespace types {
      typedef struct ComplexYoloPrediction {
        float x;
        float y;
        float z;
        float ry;
        float cls_pred;
      } ComplexYoloPrediction;
  } // namespace types
} // namespace mxre

#endif

