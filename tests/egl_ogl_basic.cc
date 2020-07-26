#include <mxre.h>

#include <EGL/egl.h>
#include <GL/glut.h>
#include <iostream>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[])
{
  mxre::eglutils::EGLPbuffer pbuf;
  mxre::eglutils::initEGLPbuffer(pbuf);
  mxre::eglutils::setCurrentPbuffer(pbuf);

  mxre::glutils::initGL(WIDTH, HEIGHT);

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  gluLookAt(0, 0, 0,
            0, 0, 0,
            0, 1, 0);

  // MODELVIEW reset
  glPushMatrix();
  glLoadIdentity();

  // Projection reset
  glMatrixMode(GL_PROJECTION);                 // switch to projection matrix
  glPushMatrix();                              // save current projection matrix
  glLoadIdentity();                            // reset projection matrix
  //gluOrtho2D(0, WIDTH, 0, HEIGHT); // set to orthogonal projection

  glBegin(GL_QUADS);
  glColor3f(1, 0, 0);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();


  //glTranslatef(0.0f, 0.0f, -6.0f);  // Move right and into the screen
  glBegin(GL_QUADS);
  glColor3f(0, 1, 0);
  glVertex3f(0.5f, 0.5f, -1.0f);
  glVertex3f(-0.5f, 0.5f, -1.0f);
  glVertex3f(-0.5f, -0.5f, -1.0f);
  glVertex3f(0.5f, -0.5f, -1.0f);
  glEnd();

  glPopMatrix();

  cv::Mat cv_data = mxre::glutils::exportGLBufferToCV();
  cv::imwrite("result.jpg", cv_data);

  mxre::eglutils::terminatePbuffer(pbuf);
  return 0;
}