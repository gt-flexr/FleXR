macro(kernel_orbslam_pose_estimator)
  if(${KERNEL_ORBSLAM_POSE_ESTIMATOR})
    find_package(Pangolin REQUIRED)
    pkg_search_module(EIGEN3 REQUIRED eigen3)
    pkg_search_module(G2O  REQUIRED g2o)
    pkg_search_module(DBoW REQUIRED dbow2)
    pkg_search_module(ORBSLAM3 REQUIRED orbslam3)

    include_directories(${OPENCV_INCLUDE_DIRS})
    list(APPEND FLEXR_KERNEL_CXX_FLAGS      ${EIGEN3_CXX_FLAGS})
    list(APPEND FLEXR_KERNEL_INCLUDE_DIRS -I${EIGEN3_INCLUDE_DIRS})
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS   ${EIGEN3_LDFLAGS})

    list(APPEND FLEXR_KERNEL_CXX_FLAGS      ${G2O_CXX_FLAGS})
    list(APPEND FLEXR_KERNEL_INCLUDE_DIRS -I${G2O_INCLUDE_DIRS})
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS   ${G2O_LDFLAGS})

    list(APPEND FLEXR_KERNEL_CXX_FLAGS      ${DBoW_CXX_FLAGS})
    list(APPEND FLEXR_KERNEL_INCLUDE_DIRS -I${DBoW_INCLUDE_DIRS})
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS   ${DBoW_LDFLAGS})

    list(APPEND FLEXR_KERNEL_CXX_FLAGS      ${ORBSLAM3_CXX_FLAGS})
    list(APPEND FLEXR_KERNEL_INCLUDE_DIRS -I${ORBSLAM3_INCLUDE_DIRS})
    list(APPEND FLEXR_KERNEL_LINKER_FLAGS   ${ORBSLAM3_LDFLAGS})

    include_directories(${EIGEN3_INCLUDE_DIRS})
    include_directories(${G2O_INCLUDE_DIRS})
    include_directories(${DBoW_INCLUDE_DIRS})
    include_directories(${ORBSLAM3_INCLUDE_DIRS})

    add_definitions(-D__FLEXR_KERNEL_ORBSLAM_POSE_ESTIMATOR__)
    list(APPEND FLEXR_KERNEL_CXX_FLAGS -D__FLEXR_KERNEL_ORBSLAM_POSE_ESTIMATOR__)
    message("\t [Intermediate] OrbSlamPoseEstimator")


  endif()
endmacro()

