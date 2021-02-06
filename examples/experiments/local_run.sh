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
MARKER_PATH="$MXRE_DIR/resources/markers/"
IMG_PATH_1080="$MXRE_DIR/resources/video/1080/video_000120.png"

WIDTH=1920
HEIGHT=1080

cd $BUILD_PATH && cmake .. && make -j4
cd $EXE_PATH
echo $(pwd)

timelimit $EXE_TIME_CPU ./serialized_mock_cam $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080
timelimit $EXE_TIME_GPU ./serialized_mock_cam_cuda $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080

echo "timeout -sHUP $EXE_TIME_CPU ./mock_cam_adq $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080"
timelimit $EXE_TIME_CPU ./mock_cam_adq $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080
timelimit $EXE_TIME_GPU ./mock_cam_cuda_adq $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080

for QUEUE_SIZE in 1 5 9
do
  timelimit $EXE_TIME_CPU ./mock_cam_q $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080 $QUEUE_SIZE
  timelimit $EXE_TIME_GPU ./mock_cam_cuda_q $WIDTH $HEIGHT $MARKER_PATH $IMG_PATH_1080 $QUEUE_SIZE
done

