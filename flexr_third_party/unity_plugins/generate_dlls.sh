# Test plugins
g++ FaceDetection.cc -O3 -fPIC -shared -o FaceDetection.so `pkg-config --libs --cflags opencv`
g++ CameraInput.cc -O3 -fPIC -shared -o CameraInput.so `pkg-config --libs --cflags opencv`

# FleXR integration
g++ FleXRPlugin.cc -O3 -fPIC -shared -o FleXRPlugin.so `pkg-config --libs --cflags opencv libzmq`
g++ flexr_rtp_plugin.cc -fPIC -shared -o FlexrRtpPlugin.so `pkg-config --libs --cflags opencv4 uvgrtp` -I/home/jin/mnt/github/flexr

