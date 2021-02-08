# Test plugins
g++ FaceDetection.cc -O3 -fPIC -shared -o FaceDetection.so `pkg-config --libs --cflags opencv`
g++ CameraInput.cc -O3 -fPIC -shared -o CameraInput.so `pkg-config --libs --cflags opencv`

# MXRE integration
g++ MXREPlugin.cc -O3 -fPIC -shared -o MXREPlugin.so `pkg-config --libs --cflags opencv libzmq`
