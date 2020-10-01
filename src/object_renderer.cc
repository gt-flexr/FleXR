#include "object_renderer.h"
#include <GL/gl.h>

namespace mxre
{
  namespace pipeline
  {
    namespace rendering
    {
      ObjectRenderer::ObjectRenderer(std::vector<mxre::cv_units::ObjectInfo> registeredObjs) :
        camera(glm::vec3(0.0f, 3.0f, 0.0f)), raft::kernel()
      {
        isSet = false;
        mxre::egl::initEGLPbuffer(pbuf);

        input.addPort<mxre::cv_units::Mat>("in_frame");
        input.addPort<std::vector<mxre::gl::ObjectContext>>("in_obj_context");

        output.addPort<mxre::cv_units::Mat>("out_frame");

        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for(objIter = registeredObjs.begin(); objIter != registeredObjs.end(); ++objIter) {
          ModelMap newObjMap;
          newObjMap.objectIndex = objIter->index;
          newObjMap.modelMat.push_back(glm::mat4(1.0f));
          modelMaps.push_back(newObjMap);
        }

#ifdef __PROFILE__
        input.addPort<FrameStamp>("frame_stamp");
        output.addPort<FrameStamp>("frame_stamp");
#endif
      }


      ObjectRenderer::~ObjectRenderer() {
        mxre::egl::terminatePbuffer(pbuf);
      }


      raft::kstatus ObjectRenderer::run()
      {
        mxre::egl::setCurrentPbuffer(pbuf);
        mxre::gl::initGL(WIDTH, HEIGHT);
        if(!isSet) {
          shader.init(mxre::utils::PathFinder::find("build/examples/model_loading.vs").c_str(),
              mxre::utils::PathFinder::find("build/examples/model_loading.fs").c_str());

          std::vector<ModelMap>::iterator mapIter;
          for(mapIter = modelMaps.begin(); mapIter != modelMaps.end(); ++mapIter) {
            mxre::gl::Model model(mxre::utils::PathFinder::find("resources/obj_neck_mech/Neck_Mech_Walker.obj"));
            mapIter->models.push_back(model);
          }

          stbi_set_flip_vertically_on_load(true);
          isSet = true;
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
        if(glIsTexture(backgroundTexture)) {
          mxre::gl::updateTextureFromCVFrame(frame, backgroundTexture);
        }
        else
          mxre::gl::makeTextureFromCVFrame(frame, backgroundTexture);
        frame.release();

        // 2. Draw background frame
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mxre::gl::startBackground(WIDTH, HEIGHT);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
          glTexCoord2i(0,0); glVertex3f(0, 0, -1);
          glTexCoord2i(1,0); glVertex3f(WIDTH, 0, -1);
          glTexCoord2i(1,1); glVertex3f(WIDTH, HEIGHT, -1);
          glTexCoord2i(0,1); glVertex3f(0, HEIGHT, -1);
        glEnd();
        mxre::gl::endBackground();

        shader.use();
        //glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)800/(float)600, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        std::vector<mxre::gl::ObjectContext>::iterator objCtxIter;
        for (objCtxIter = objCtxVec.begin(); objCtxIter != objCtxVec.end(); ++objCtxIter)
        {
          glm::mat4 modelMat = glm::mat4(1.0f);

          modelMat = glm::translate(modelMat, objCtxIter->tvec);
          modelMat = glm::rotate(modelMat, objCtxIter->rvec[0], glm::vec3(1, 0, 0));
          modelMat = glm::rotate(modelMat, objCtxIter->rvec[1], glm::vec3(0, 1, 0));
          modelMat = glm::rotate(modelMat, objCtxIter->rvec[2], glm::vec3(0, 0, 1));
          modelMat = glm::scale(modelMat, glm::vec3(0.3f, 0.3f, 0.3f));
          shader.setMat4("model", modelMat);
          modelMaps[0].models[0].draw(shader);
          glFlush();
        }
        glPopMatrix();
        glUseProgram(0);
        out_frame = mxre::gl::exportGLBufferToCV();

        input["in_frame"].recycle();
        input["in_obj_context"].recycle();

#ifdef __PROFILE__
        TimeVal end = getNow();
        debug_print("Exe Time: %lfms", getExeTime(end, start));

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

