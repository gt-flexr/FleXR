#ifndef __GL_UTILS__
#define __GL_UTILS__

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES // for fbo extensions
#endif

#include "defs.h"
#include "cv_types.h"

#include <GL/glew.h>
#include <GL/glut.h>

#include <opencv2/opencv.hpp>
#include <iostream>

namespace mxre
{
  namespace gl
  {
    static void initLights()
    {
      // set up light colors (ambient, diffuse, specular)
      GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f}; // ambient light
      GLfloat lightKd[] = {.7f, .7f, .7f, 1.0f}; // diffuse light
      GLfloat lightKs[] = {1, 1, 1, 1};          // specular light
      glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
      glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
      glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

      // position the light
      float lightPos[4] = {0, 0, 20, 1}; // positional light
      glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

      glEnable(GL_LIGHT0); // MUST enable each light source after configuration
    }


    static void initGL(int width, int height)
    {
      glewInit();
      glShadeModel(GL_SMOOTH);               // shading mathod: GL_SMOOTH or GL_FLAT
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // 4-byte pixel alignment

      // enable /disable features
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glEnable(GL_CULL_FACE);

      // enable /disable features
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
      glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_LINE_SMOOTH);

      // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);

      glClearColor(0, 0, 0, 0); // background color
      glClearDepth(1.0f);       // 0 is near, 1 is far
      glDepthFunc(GL_LEQUAL);

      //glEnable(GL_LIGHTING);
      //initLights();

      glViewport(0, 0, (GLsizei)width, (GLsizei)height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(60.0f, (float)(width) / height, 1.0f, 100.0f); // FOV, AspectRatio, NearClip, FarClip

      // switch to modelview matrix in order to set scene
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
    }


    static mxre::cv_units::Mat exportGLBufferToCV()
    {
      glReadBuffer(GL_FRONT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      unsigned char *pixels = new unsigned char[3 * WIDTH * HEIGHT];
      glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, pixels);
      debug_print("allocated pixel addr: %p", static_cast<void*>(pixels));

      mxre::cv_units::Mat mat(HEIGHT, WIDTH, CV_8UC3, pixels);

      // flip around X-axis (Y-flip) from GL to CV
      cv::flip(mat.cvMat, mat.cvMat, 0);
      return mat;
    }


    static void makeEmptyTexture(GLuint &tex, int width, int height)
    {
      glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_2D, tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      //glGenerateMipmap(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, 0);
    }


    static void makeTextureFromCVFrame(mxre::cv_units::Mat &mat, GLuint &tex)
    {
      if (mat.cvMat.empty())
      {
        std::cout << "image empty" << std::endl;
        return;
      }

      glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_2D, tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      // flip around X-axis (Y-flip) from CV to GL
      cv::flip(mat.cvMat, mat.cvMat, 0);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mat.cols, mat.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, mat.data);
      glBindTexture(GL_TEXTURE_2D, 0);
    }


    static void updateTextureFromCVFrame(mxre::cv_units::Mat &mat, GLuint &tex)
    {
      glBindTexture(GL_TEXTURE_2D, tex);
      // flip around X-axis (Y-flip) from CV to GL
      cv::flip(mat.cvMat, mat.cvMat, 0);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mat.cols, mat.rows, GL_BGR, GL_UNSIGNED_BYTE, mat.data);
      glBindTexture(GL_TEXTURE_2D, 0);
    }


    static void startBackground(int width, int height) {
      // 1. Clear ModelView
      glPushMatrix();
      glLoadIdentity();

      // 2. Clear Projection
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();

      // 2. Set projection as orthogonal projection
      //glOrtho(-width/2, width/2, -height/2, height/2, -1, 1);

      // 3. Set GL_MODELVIEW with the orthogonal projection
      //glMatrixMode(GL_MODELVIEW);
      //glPushMatrix();
      //glLoadIdentity();
      //glTranslatef(-width/2, -height/2, 0); // set mid point
      gluOrtho2D(0, width, 0, height); // set to orthogonal projection
    }


    static void endBackground() {
      // 1. Restore the previous projection
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();

      // 2. Restore the model-view matrix
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }


    static bool checkFramebuffer(GLuint fbo)
    {
      std::cout << "FBO: " << fbo << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, fbo); // bind
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      switch (status)
      {
      case GL_FRAMEBUFFER_COMPLETE:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        std::cout << "[ERROR] Framebuffer incomplete: Multisample." << std::endl;
        return false;

      case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation." << std::endl;
        return false;

      default:
        std::cout << "[ERROR] Framebuffer incomplete: Unknown error." << std::endl;
        return false;
      }
      glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind
    }


    static void makeFramebuffer(GLuint &fboID, GLuint &rboDepthID, GLuint &tex, int width, int height)
    {
      // Frame Buffer
      glGenFramebuffers(1, &fboID);
      glBindFramebuffer(GL_FRAMEBUFFER, fboID);

      //   Render Buffer
      glGenRenderbuffers(1, &rboDepthID);
      glBindRenderbuffer(GL_RENDERBUFFER, rboDepthID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      // Texture ---> Framebuffer COLOR attachment point
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

      // Renderbuffer ---> Framebuffer DEPTH attachment point
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthID);

      bool status = checkFramebuffer(fboID);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

  } // namespace gl
} // namespace mxre

#endif
