#include <kernels/intermediate/complexyolo_renderer.h>
#include <GL/gl.h>
#include <GL/glew.h>

namespace mxre
{
  namespace kernels
  {

    ComplexYOLORenderer::ComplexYOLORenderer(int width, int height, int depth) :
        camera(glm::vec3(0.0f, -2.5f, 3.0f), glm::vec3(0, 1, 0), -90, 0, 50), width(width), height(height),
        depth(depth), worldMap(0), raft::kernel()
    {
      input.addPort<mxre::types::Frame>("in_frame");
      input.addPort<std::vector<mxre::types::ComplexYoloPrediction>>("in_obj_context");

      output.addPort<mxre::types::Frame>("out_frame");

      // 0. Create pbuf as a context
      pbuf = new mxre::egl_types::pbuffer;
      mxre::egl_utils::initPbuffer(*pbuf, width, height);

      // 1. Init pbuf as context and GL with the context
      mxre::egl_utils::bindPbuffer(*pbuf);
      mxre::gl_utils::initGL(width, height);

      // 2. Init Shader
      stbi_set_flip_vertically_on_load(true);
      shader.init(mxre::utils::PathFinder::find("build/examples/model_loading.vs").c_str(),
          mxre::utils::PathFinder::find("build/examples/model_loading.fs").c_str());

      // 3. add 3D models with textures for three classes: Car, Pedestrian, Cyclist
      //worldMap.addModel("resources/4_shuttle/Transport Shuttle_obj.obj"); // Car
      worldMap.addModel("resources/ArmyTruck_OBJ/armytruck.obj"); // Car
      worldMap.addModel("resources/0_stone/Stone.obj"); // Pedestrian
      worldMap.addModel("resources/1_neck/Neck_Mech_Walker.obj"); // Cyclist

      // 5. Unbind the pbuf context in init thread
      mxre::egl_utils::unbindPbuffer(*pbuf);
      binding = false;
    }


    ComplexYOLORenderer::~ComplexYOLORenderer() {
      mxre::egl_utils::terminatePbuffer(*pbuf);
      delete [] pbuf;
    }


    raft::kstatus ComplexYOLORenderer::run()
    {
      if(!binding) {
        mxre::egl_utils::bindPbuffer(*pbuf);
        binding = true;
      }
      // get inputs from the previous kernel: ObjectDetector
      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );
      auto objCtxVec( input["in_obj_context"].peek<std::vector<mxre::types::ComplexYoloPrediction>>() );

      for(unsigned int i = 0; i < objCtxVec.size(); i++) {
        printf("Object: %f %f %f %f %f\n", objCtxVec[i].cls_pred, objCtxVec[i].x, objCtxVec[i].y, objCtxVec[i].z,
            objCtxVec[i].ry);
      }

      // set outputs
      auto &out_frame( output["out_frame"].allocate<mxre::types::Frame>() );

      // 1. Create/update background texture & release previous CV frame
      if(glIsTexture(backgroundTexture))
        mxre::gl_utils::updateTextureFromFrame(&frame, backgroundTexture);
      else
        mxre::gl_utils::makeTextureFromFrame(&frame, backgroundTexture);
      frame.release();


      // 2. Add objects to the world
      // 2.1. iterate the detected objects
      // 2.2. add objects with model indices 0: Car, 1: Pedestrian, 2: Cyclist
      for (unsigned int i = 0; i < objCtxVec.size(); i++)
      {
        int clsPred = (int)objCtxVec[i].cls_pred;
        float normX = objCtxVec[i].x / width;
        float scaledX = 0, scaledY = 0, scaledZ = 0;

        float normY = objCtxVec[i].y / height;

        double scaleFactor = 0;
        float normZ = objCtxVec[i].z / depth;

        float ry = objCtxVec[i].ry;

        if(clsPred == 0) { // Car
          scaleFactor = 0.025;
          scaledX = (normX * 24) - 12;
          scaledY = (normY * 8) - 6.5;
          scaledZ = (normZ * 8) + 1;

          if(ry >= 315 || ry < 45) ry = -80;
          else if(ry >= 45 && ry < 135) ry = 0;
          else if(ry >= 135 && ry < 225) ry = 80;
          else ry = 160;
        }
        else if(clsPred == 1) { // Pedestrian
          scaleFactor = 0.3;
          scaledX = (normX * 16) - 8;
          scaledY = (normY * 5) - 5;
          scaledZ = (normZ * 3) + 1;

          if(ry >= 315 || ry < 45) ry = -80;
          else if(ry >= 45 && ry < 135) ry = 0;
          else if(ry >= 135 && ry < 225) ry = 80;
          else ry = 160;
        }
        else if(clsPred == 2) { // Cyclist
          scaleFactor = 0.4;
          scaledX = (normX * 16) - 8;
          scaledY = (normY * 5) - 5;
          scaledZ = (normZ * 3) + 1;

          if(ry >= 315 || ry < 45) ry = 0;
          else if(ry >= 45 && ry < 135) ry = 80;
          else if(ry >= 135 && ry < 225) ry = -160;
          else ry = -80;
        }

        mxre::ar_types::VirtualObject newObj(clsPred);
        newObj.translate(glm::vec3(scaledX, scaledY, -(scaledZ)));
        newObj.scale(glm::vec3(scaleFactor/scaledZ, scaleFactor/scaledZ, scaleFactor/scaledZ));
        newObj.rotate(glm::vec3(0, ry, 0));

        worldMap.addObject(newObj);
      }

      // 2. Draw background frame
      glClearColor(1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      mxre::gl_utils::startBackground(width, height);
      glBindTexture(GL_TEXTURE_2D, backgroundTexture);
      glBegin(GL_QUADS);
      glColor3f(1, 1, 1);
        glTexCoord2i(0,0); glVertex3f(0,              0,        -1);
        glTexCoord2i(1,0); glVertex3f(width,          0,        -1);
        glTexCoord2i(1,1); glVertex3f(width,     height,        -1);
        glTexCoord2i(0,1); glVertex3f(0,         height,        -1);
      glEnd();
      mxre::gl_utils::endBackground();

      shader.use();
      glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)width/(float)height, 0.1f, 100.0f);
      glm::mat4 view = camera.getViewMatrix();
      shader.setMat4("projection", projection);
      shader.setMat4("view", view);

      worldMap.resetCoord();
      worldMap.draw(shader);
      glFlush();

      glPopMatrix();
      glUseProgram(0);
      worldMap.emptyObject();
      out_frame = mxre::gl_utils::exportGLBufferToCV(width, height, 0, 0); // IMGSIZE!!!!

      input["in_frame"].recycle();
      input["in_obj_context"].recycle();

      output["out_frame"].send();
      return raft::proceed;
    }

  }   // namespace kernels
} // namespace mxre

