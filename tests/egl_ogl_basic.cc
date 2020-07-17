#include <opencv4/opencv2/opencv.hpp>
#include <EGL/egl.h>
#include <GL/glut.h>
#include <iostream>
#include <stdio.h>

using namespace std;

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE};

static const int pbufferWidth = 1280;
static const int pbufferHeight = 640;
static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, pbufferWidth,
    EGL_HEIGHT, pbufferHeight,
    EGL_NONE,
};

cv::Mat capture()
{
  glReadBuffer(GL_FRONT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  unsigned char *pixels = new unsigned char[3 * 1280 * 640];
  glReadPixels(0, 0, 1280, 640, GL_BGR, GL_UNSIGNED_BYTE, pixels);

  return cv::Mat(640, 1280, CV_8UC3, pixels);
}

int main(int argc, char *argv[])
{
  // 1. Initialize EGL
  EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  EGLint major, minor;
  EGLint test;
  GLubyte *data = (GLubyte*)malloc(1280 * 640 * 3);
  //GLubyte *data = new GLubyte[1280*640*3];

  eglInitialize(eglDpy, &major, &minor);

  // 2. Select an appropriate configuration
  EGLint numConfigs;
  EGLConfig eglCfg;

  eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

  // 3. Create a surface
  EGLSurface eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg,
                                               pbufferAttribs);

  // 4. Bind the API
  eglBindAPI(EGL_OPENGL_API);

  // 5. Create a context and make it current
  EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT,
                                       NULL);

  eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);

  // from now on use your OpenGL context
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  eglSwapBuffers(eglDpy, eglSurf);
  //glReadPixels(0, 0, 1280, 640, GL_RGB, GL_UNSIGNED_BYTE, data);
  cv::Mat cv_data = capture();
  cv::imwrite("result.jpg", cv_data);

  eglQuerySurface(eglDpy, eglSurf, EGL_WIDTH, &test);
  cout << "Surface WIDTH: " << test << endl;
  eglQuerySurface(eglDpy, eglSurf, EGL_HEIGHT, &test);
  cout << "Surface HEIGHT: " << test << endl;

  // 6. Terminate EGL when finished
  eglTerminate(eglDpy);
  return 0;
}