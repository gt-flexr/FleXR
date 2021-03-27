# ./EXE WIDTH HEIGHT MARKER_PATH IMG_PATH [QUEUE_SIZE]

EXE_TIME_CPU='-t5'
EXE_TIME_GPU='-t5'

if [ -z "${MXRE_HOME}" ]
then
  echo "\tSet MXRE_HOME as an environment variable"
  exit 0
fi

MXRE_DIR=${MXRE_HOME}
BUILD_PATH="$MXRE_DIR/build"
EXE_PATH="$BUILD_PATH/examples"

cd $BUILD_PATH && cmake .. && make -j4
cd $EXE_PATH
echo $(pwd)

timelimit $EXE_TIME_CPU ./serialized_mock_cam
timelimit $EXE_TIME_GPU ./serialized_mock_cam_cuda

timelimit $EXE_TIME_CPU ./mock_cam_adq
timelimit $EXE_TIME_GPU ./mock_cam_cuda_adq

timelimit $EXE_TIME_CPU ./mock_cam_q
timelimit $EXE_TIME_GPU ./mock_cam_cuda_q

