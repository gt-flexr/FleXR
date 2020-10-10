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
        width(width), height(height), raft::kernel()
      {
        // ObjectCtxExtractor inputs
        input.addPort<mxre::cv_units::Mat>("in_frame");
        input.addPort<std::vector<mxre::gl::ObjectContext>>("in_obj_context");

        // Keyboard input
        input.addPort<char>("in_keystroke");

        output.addPort<mxre::cv_units::Mat>("out_frame");

        // 0. Create pbuf as a context
        pbuf = new mxre::egl::EGLPbuffer;
        mxre::egl::initEGLPbuffer(*pbuf, width, height);

        // 1. Init pbuf as context and GL with the context
        mxre::egl::bindPbuffer(*pbuf);
        mxre::gl::initGL(width, height);

        // 2. Set AR Worlds via WorldManager
        // 2.1. Init shader
        worldManager.initShader();
        // 2.2. Add new worlds
        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for(objIter = registeredObjs.begin(); objIter != registeredObjs.end(); ++objIter) {
          worldManager.addWorld(objIter->index);
        }
        // 2.3. Add an object to each world (temporarily)
        for(int i = 0; i < worldManager.numOfWorlds; i++)
          worldManager.addObjectToWorld(i);

        // 3. Unbind the pbuf context in init thread
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

        // 0.0.Get inputs from the previous kernel: ObjectDetector
        auto &frame( input["in_frame"].peek<mxre::cv_units::Mat>() );
        auto objCtxVec( input["in_obj_context"].peek<std::vector<mxre::gl::ObjectContext>>() );

        // 0.1.Get input keystroke from Keyboard
        auto &keyPort(input["in_keystroke"]);
        char key;
        if(keyPort.size() > 0) {
          key = input ["in_keystroke"].peek<char>();
          std::cout << "Input Key captured object_renderer: " << key << std::endl;
          keyPort.recycle(1);
        }
        else key = 0;

        // 0.2.Set output
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

        worldManager.startWorlds(key, objCtxVec);

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

