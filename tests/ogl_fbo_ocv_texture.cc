#include <mxre.h>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <chrono>
#include <iostream>

using namespace std;
using namespace cv;

void printMatInfo(mxre::gltypes::Matrix4 &mat, string matName)
{
  std::cout << "=== Matrix Info: " << matName << " ===" << std::endl;

  std::cout << "[" << std::setw(8) << mat[0] << std::setw(8) << mat[4] << std::setw(8) << mat[8] << std::setw(8) << mat[12] << "]" << std::endl;
  std::cout << "[" << std::setw(8) << mat[1] << std::setw(8) << mat[5] << std::setw(8) << mat[9] << std::setw(8) << mat[13] << "]" << std::endl;
  std::cout << "[" << std::setw(8) << mat[2] << std::setw(8) << mat[6] << std::setw(8) << mat[10] << std::setw(8) << mat[14] << "]" << std::endl;
  std::cout << "[" << std::setw(8) << mat[3] << std::setw(8) << mat[7] << std::setw(8) << mat[11] << std::setw(8) << mat[15] << "]" << std::endl << std::endl;
}

void orthogonalStart()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-WIDTH/2, WIDTH/2, -HEIGHT/2, HEIGHT/2);
    glMatrixMode(GL_MODELVIEW);
}

void orthogonalEnd()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawBox()
{
  glBegin(GL_QUADS); // Begin drawing the color cube with 6 quads
  glColor3f(0.0f, 1.0f, 0.0f); // Green
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);

  // Bottom face (y = -1.0f)
  glColor3f(1.0f, 0.5f, 0.0f); // Orange
  glVertex3f(1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);

  // Front face  (z = 1.0f)
  glColor3f(1.0f, 0.0f, 0.0f); // Red
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);

  // Back face (z = -1.0f)
  glColor3f(1.0f, 1.0f, 0.0f); // Yellow
  glVertex3f(1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, -1.0f);

  // Left face (x = -1.0f)
  glColor3f(0.0f, 0.0f, 1.0f); // Blue
  glVertex3f(-1.0f, 1.0f, 1.0f);
  glVertex3f(-1.0f, 1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, -1.0f);
  glVertex3f(-1.0f, -1.0f, 1.0f);

  // Right face (x = 1.0f)
  glColor3f(1.0f, 0.0f, 1.0f); // Magenta
  glVertex3f(1.0f, 1.0f, -1.0f);
  glVertex3f(1.0f, 1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, 1.0f);
  glVertex3f(1.0f, -1.0f, -1.0f);
  glEnd(); // End of drawing color-cube
}

int main(int argc, char **argv)
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
  mxre::glutils::initGL(WIDTH, HEIGHT);

  GLuint textureID;
  mxre::glutils::makeTextureFromCVFrame(img, textureID);

  chrono::system_clock::time_point st = chrono::high_resolution_clock::now();
  int iteration = 1000;
  for(int i = 0; i < iteration; i++) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    gluLookAt (0.0, 0.0, 5.0,
               0.0, 0.0, 0.0,
               0.0, 1.0, 0.0);

    mxre::glutils::startBackground(WIDTH, HEIGHT);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glBegin(GL_QUADS);
    glColor3f(1, 1, 1);
      glTexCoord2i(0,0); glVertex3f(-1.0f, -1.0f, 1.0f);
      glTexCoord2i(1,0); glVertex3f(1.0f, -1.0f, 1.0f);
      glTexCoord2i(1,1); glVertex3f(1.0f, 1.0f, 1.0f);
      glTexCoord2i(0,1); glVertex3f(-1.0f, 1.0f, 1.0f);
    glEnd();
    mxre::glutils::endBackground();

    // 모델 매트릭스 -- 회전 -- 직육면체 그려서 배경인지 확인
    //glRotatef(30, 1, 0, 0);
    glRotatef(30, 0, 1, 0);
    glRotatef(30, 0, 0, 1);
    drawBox();

    glFlush();
    glPopMatrix();
  }

  chrono::system_clock::time_point et = chrono::high_resolution_clock::now();
  float et_ms = float(chrono::duration_cast<chrono::microseconds>(
                      et - st).count()) / 1000;
  cout << "Total Frame Time: " << et_ms << " ms" << endl;
  cout << "Each Frame Time: " << et_ms / iteration << " ms" << endl;

  cv::Mat cv_data = mxre::glutils::exportGLBufferToCV();
  cv::imwrite("result.jpg", cv_data);

  mxre::eglutils::terminatePbuffer(pbuf);
  return 0;             /* ANSI C requires main to return int. */
}