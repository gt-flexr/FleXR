#include <opencv2/opencv.hpp>

#include <mxre.h>
#include <EGL/egl.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>


#include <iostream>
#include <chrono>
#include <stdio.h>

using namespace std;
using namespace cv;

void initGL(void)
{
  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  gluPerspective(40.0, // FoV Degree
                 1.0, // aspect ratio
                 1.0, // Z near
                 10.0 // Z far
                 );
  glMatrixMode(GL_MODELVIEW);
}



int main(int argc, char *argv[])
{
  Mat img = imread(argv[1]);
  if (img.empty())
  {
    cout << "Cannot load image: " << argv[1] << endl;
    exit(EXIT_FAILURE);
  }

  mxre::eglutils::EGLPbuffer pbuf;
  mxre::eglutils::initEGLPbuffer(pbuf);
  mxre::eglutils::setCurrentPbuffer(pbuf);

  // from now on use your OpenGL context
  initGL();

  // 1. Get Texture
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  GLuint textureID;
  mxre::glutils::makeTextureFromCVFrame(img, textureID);

  // 2. Draw Texture
  eglBindTexImage(pbuf.eglDpy, pbuf.eglSurf, EGL_BACK_BUFFER);
  eglSwapBuffers(pbuf.eglDpy, pbuf.eglSurf);
  glBegin(GL_QUADS);
  glTexCoord2i(0, 0);
  glVertex2i(0, 0);
  glTexCoord2i(0, 1);
  glVertex2i(0, HEIGHT);
  glTexCoord2i(1, 1);
  glVertex2i(WIDTH, HEIGHT);
  glTexCoord2i(1, 0);
  glVertex2i(WIDTH, 0);
  glEnd();

  glDeleteTextures(1, &textureID);
  glDisable(GL_TEXTURE_2D);

  glFlush();

  // check EGL pbuffer result
  cv::Mat cv_data = mxre::glutils::exportGLBufferToCV();
  cv::imwrite("result.jpg", cv_data);

  // 6. Terminate EGL when finished
  mxre::eglutils::terminatePbuffer(pbuf);
  return 0;
}