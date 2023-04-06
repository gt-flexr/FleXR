# ./docker_run.sh CONTAINER_NAME IMAGE
# ex) ./docker_run.sh flexr_cpu jheo4/flexr:base_2004

docker run -it \
  --net=host \
  --ipc=host \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix/:/tmp/.X11-unix/ \
  --privileged \
  --name $1 $2

