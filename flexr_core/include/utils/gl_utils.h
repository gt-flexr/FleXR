#ifndef __FLEXR_CORE_GL_UTILS__
#define __FLEXR_CORE_GL_UTILS__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "flexr_core/include/defs.h"

namespace flexr
{
  namespace gl_utils
  {
    // OpenCV RGB - OpenGL BGR
    static GLuint bgrMatToRgbTexture(const cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
    {
      // Generate a number for our textureID's unique handle
      GLuint textureID;
      glGenTextures(1, &textureID);

      // Bind to our texture handle
      glBindTexture(GL_TEXTURE_2D, textureID);

      // Catch silly-mistake texture interpolation method for magnification
      if (magFilter == GL_LINEAR_MIPMAP_LINEAR  ||
          magFilter == GL_LINEAR_MIPMAP_NEAREST ||
          magFilter == GL_NEAREST_MIPMAP_LINEAR ||
          magFilter == GL_NEAREST_MIPMAP_NEAREST)
      {
        debug_print("You can't use MIPMAPs for magnification - setting filter to GL_LINEAR");
        magFilter = GL_LINEAR;
      }

      // Set texture interpolation methods for minification and magnification
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

      // Set texture clamping method
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

      // Set incoming texture format to:
      // GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
      // GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
      // Work out other mappings as required ( there's a list in comments in main() )
      GLenum inputColourFormat = GL_BGR;
      if (mat.channels() == 1)
      {
        inputColourFormat = GL_LUMINANCE;
      }

      // Create the texture
      glTexImage2D(GL_TEXTURE_2D,     // Type of texture
          0,                 // Pyramid level (for mip-mapping) - 0 is the top level
          GL_RGB,            // Internal colour format to convert to
          mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
          mat.rows,          // Image height i.e. 480 for Kinect in standard mode
          0,                 // Border width in pixels (can either be 1 or 0)
          inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
          GL_UNSIGNED_BYTE,  // Image data type
          mat.ptr());        // The actual image data itself

      // If we're using mipmaps then generate them. Note: This requires OpenGL 3.0 or higher
      if (minFilter == GL_LINEAR_MIPMAP_LINEAR  ||
          minFilter == GL_LINEAR_MIPMAP_NEAREST ||
          minFilter == GL_NEAREST_MIPMAP_LINEAR ||
          minFilter == GL_NEAREST_MIPMAP_NEAREST)
      {
        glGenerateMipmap(GL_TEXTURE_2D);
      }

      return textureID;
    }


    static void error_callback(int error, const char* description)
    {
      fprintf(stderr, "Error: %s\n", description);
    }


    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
      if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      }
    }


    static void resize_callback(GLFWwindow* window, int new_width, int new_height)
    {
      glViewport(0, 0, new_width, new_height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0.0, new_width, new_height, 0.0, 0.0, 100.0);
      glMatrixMode(GL_MODELVIEW);
    }


    static void drawBgrCvToRgbGlWnd(const cv::Mat& frame, int width, int height)
    {
      // Clear color and depth buffers
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glMatrixMode(GL_MODELVIEW);     // Operate on model-view matrix

      glEnable(GL_TEXTURE_2D);
      GLuint image_tex = bgrMatToRgbTexture(frame, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP);

      /* Draw a quad */
      glBegin(GL_QUADS);
      glTexCoord2i(0, 0); glVertex2i(0,   0);
      glTexCoord2i(0, 1); glVertex2i(0,   height);
      glTexCoord2i(1, 1); glVertex2i(width, height);
      glTexCoord2i(1, 0); glVertex2i(width, 0);
      glEnd();

      glDeleteTextures(1, &image_tex);
      glDisable(GL_TEXTURE_2D);
    }


    static void init_opengl(int w, int h)
    {
      glViewport(0, 0, w, h); // use a screen size of WIDTH x HEIGHT

      glMatrixMode(GL_PROJECTION);     // Make a simple 2D projection on the entire window
      glLoadIdentity();
      glOrtho(0.0, w, h, 0.0, 0.0, 100.0);

      glMatrixMode(GL_MODELVIEW);    // Set the matrix mode to object modeling

      glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      glClearDepth(0.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the window
    }

  } // namespace gl_utils
} // namespace flexr

#endif

