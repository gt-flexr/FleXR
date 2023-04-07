# ./docker_run_gpu.sh CONTAINER_NAME IMAGE
# ex) ./docker_run_gpu.sh flexr_cpu jheo4/flexr:nv_base_2004_114

docker run -it \
  --net=host \
  --ipc=host \
  --gpus all \
  -e NVIDIA_DRIVER_CAPABILITIES=all \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix/:/tmp/.X11-unix/ \
  -v ~/.ssh:/home/user/.ssh:ro \
  --privileged \
  --name $1 $2

