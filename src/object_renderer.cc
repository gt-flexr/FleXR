#include "object_renderer.h"
#include <GL/gl.h>
#include <GL/glew.h>

namespace mxre
{
  namespace pipeline
  {
    namespace rendering
    {
      ObjectRenderer::ObjectRenderer(std::vector<mxre::cv_units::ObjectInfo> registeredObjs, int width, int height) :
        width(width), height(height),
        camera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0, 1, 0), -90, 0, 45), raft::kernel()
      {
        input.addPort<mxre::cv_units::Mat>("in_frame");
        input.addPort<std::vector<mxre::gl::ObjectContext>>("in_obj_context");

        output.addPort<mxre::cv_units::Mat>("out_frame");

        // 0. Create pbuf as a context
        pbuf = new mxre::egl::EGLPbuffer;
        mxre::egl::initEGLPbuffer(*pbuf, width, height);

        // 1. Init pbuf as context and GL with the context
        mxre::egl::bindPbuffer(*pbuf);
        mxre::gl::initGL(width, height);

        // 2. Init Shader
        stbi_set_flip_vertically_on_load(true);
        shader.init(mxre::utils::PathFinder::find("build/examples/model_loading.vs").c_str(),
            mxre::utils::PathFinder::find("build/examples/model_loading.fs").c_str());

        // 3. Based on the registeredObjs, set AR worlds
        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for(objIter = registeredObjs.begin(); objIter != registeredObjs.end(); ++objIter) {
          mxre::ar::World newWorld(objIter->index);
          // Register 3D models to each world
          printf("%dth world: register models", objIter->index);
          newWorld.addModel("resources/ArmyTruck_OBJ/armytruck.obj");
          newWorld.addModel("resources/1_neck/Neck_Mech_Walker.obj");
          newWorld.addModel("resources/0_stone/Stone.obj");
          newWorld.addModel("resources/2_mars/Mars 2K.obj");
          newWorld.addModel("resources/3_earth/Earth 2K.obj");
          newWorld.addModel("resources/4_shuttle/Transport Shuttle_obj.obj");

          worldMaps.push_back(newWorld);
        }

        // 4. Add objects to the worlds
        std::vector<mxre::ar::World>::iterator worldIter;
        for (worldIter = worldMaps.begin(); worldIter != worldMaps.end(); ++worldIter)
        {
          unsigned int randModelIndex = rand() % worldIter->models.size();
          mxre::ar::Object newObj(randModelIndex);
          worldIter->addObject(newObj);
        }

        // 5. Unbind the pbuf context in init thread
        mxre::egl::unbindPbuffer(*pbuf);
        binding = false;

#ifdef __PROFILE__
        input.addPort<FrameStamp>("frame_stamp");
        output.addPort<FrameStamp>("frame_stamp");
#endif
      }


      ObjectRenderer::~ObjectRenderer() {
        mxre::egl::terminatePbuffer(*pbuf);
        delete [] pbuf;
      }


      raft::kstatus ObjectRenderer::run()
      {
        if(!binding) {
          mxre::egl::bindPbuffer(*pbuf);
          binding = true;
        }

#ifdef __PROFILE__
        TimeVal start = getNow();
#endif

        // get inputs from the previous kernel: ObjectDetector
        auto &frame( input["in_frame"].peek<mxre::cv_units::Mat>() );
        auto objCtxVec( input["in_obj_context"].peek<std::vector<mxre::gl::ObjectContext>>() );

        // set outputs
        auto &out_frame( output["out_frame"].allocate<mxre::cv_units::Mat>() );

        // 1. Create/update background texture & release previous CV frame
        if(glIsTexture(backgroundTexture))
          mxre::gl::updateTextureFromCVFrame(frame, backgroundTexture);
        else
          mxre::gl::makeTextureFromCVFrame(frame, backgroundTexture);
        frame.release();

        // 2. Draw background frame
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mxre::gl::startBackground(width, height);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
          glTexCoord2i(0,0); glVertex3f(0,     0,      -1);
          glTexCoord2i(1,0); glVertex3f(width, 0,      -1);
          glTexCoord2i(1,1); glVertex3f(width, height, -1);
          glTexCoord2i(0,1); glVertex3f(0,     height, -1);
        glEnd();
        mxre::gl::endBackground();

        shader.use();
        //glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)width/(float)height, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        std::vector<mxre::gl::ObjectContext>::iterator objCtxIter;
        for (objCtxIter = objCtxVec.begin(); objCtxIter != objCtxVec.end(); ++objCtxIter)
        {
          // world iteration
          // set world mat
          // draw objects in the world
          worldMaps[objCtxIter->index].resetCoord();
          worldMaps[objCtxIter->index].translate(
              glm::vec3(objCtxIter->tvec.x, objCtxIter->tvec.y, objCtxIter->tvec.z));
          worldMaps[objCtxIter->index].rotate(glm::vec3(objCtxIter->rvec.x, objCtxIter->rvec.y, objCtxIter->rvec.z));
          printf("Translation: %f %f %f\n", objCtxIter->tvec.x, objCtxIter->tvec.y, objCtxIter->tvec.z);
          printf("Rotation: %f %f %f\n", objCtxIter->rvec.x, objCtxIter->rvec.y, objCtxIter->rvec.z);
          worldMaps[objCtxIter->index].scale(glm::vec3(0.1, 0.1, 0.1));
          worldMaps[objCtxIter->index].draw(shader);
          glFlush();
        }
        glPopMatrix();
        glUseProgram(0);
        out_frame = mxre::gl::exportGLBufferToCV(width, height);

        input["in_frame"].recycle();
        input["in_obj_context"].recycle();

#ifdef __PROFILE__
        TimeVal end = getNow();
        profile_print("Exe Time: %lfms", getExeTime(end, start));

        auto &inFrameStamp( input["frame_stamp"].peek<FrameStamp>() );
        auto &outFrameStamp( output["frame_stamp"].allocate<FrameStamp>() );
        outFrameStamp = inFrameStamp;
        input["frame_stamp"].recycle();
        output["frame_stamp"].send();
#endif

        output["out_frame"].send();
        return raft::proceed;
      }

    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre

