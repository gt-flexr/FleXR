#include <kernels/app/object_renderer.h>
#include <GL/gl.h>
#include <GL/glew.h>

namespace mxre
{
  namespace kernels
  {

    /* Constructor */
    ObjectRenderer::ObjectRenderer(std::vector<mxre::cv_types::ObjectInfo> registeredObjs, int width, int height) :
      MXREKernel(), width(width), height(height)
    {
      addInputPort<mxre::types::Frame>("in_frame");
      addInputPort<std::vector<mxre::gl_types::ObjectContext>>("in_obj_context");
      addInputPort<char>("in_keystroke");
      addOutputPort<mxre::types::Frame>("out_frame");

      // 0. Create pbuf as a context
      pbuf = new mxre::egl_types::pbuffer;
      mxre::egl_utils::initPbuffer(*pbuf, width, height);

      // 1. Init pbuf as context and GL with the context
      mxre::egl_utils::bindPbuffer(*pbuf);
      mxre::gl_utils::initGL(width, height);

      // 2. Set AR Worlds via WorldManager
      // 2.1. Init shader
      worldManager.initShader();
      // 2.2. Add new worlds
      std::vector<mxre::cv_types::ObjectInfo>::iterator objIter;
      for(objIter = registeredObjs.begin(); objIter != registeredObjs.end(); ++objIter) {
        worldManager.addWorld(objIter->index);
      }
      // 2.3. Add an object to each world (temporarily)
      for(int i = 0; i < worldManager.numOfWorlds; i++)
        worldManager.addObjectToWorld(i);

      // 3. Unbind the pbuf context in init thread
      mxre::egl_utils::unbindPbuffer(*pbuf);
      binding = false;
    }


    /* Destructor */
    ObjectRenderer::~ObjectRenderer() {
      mxre::egl_utils::terminatePbuffer(*pbuf);
      delete [] pbuf;
    }


    /* Kernel Logic */
    bool ObjectRenderer::logic(mxre::types::Frame *inFrame, std::vector<mxre::gl_types::ObjectContext> *inObjContext,
                               char inKey, mxre::types::Frame *outFrame)
    {
      // 1. Create/update background texture & release previous CV frame
      if(glIsTexture(backgroundTexture))
        mxre::gl_utils::updateTextureFromFrame(inFrame, backgroundTexture);
      else
        mxre::gl_utils::makeTextureFromFrame(inFrame, backgroundTexture);
      inFrame->release();

      // 2. Draw background frame
      glClearColor(1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      mxre::gl_utils::startBackground(width, height);
      glBindTexture(GL_TEXTURE_2D, backgroundTexture);
      glBegin(GL_QUADS);
      glColor3f(1, 1, 1);
        glTexCoord2i(0,0); glVertex3f(0,     0,      -1);
        glTexCoord2i(1,0); glVertex3f(width, 0,      -1);
        glTexCoord2i(1,1); glVertex3f(width, height, -1);
        glTexCoord2i(0,1); glVertex3f(0,     height, -1);
      glEnd();
      mxre::gl_utils::endBackground();

      worldManager.startWorlds(inKey, *inObjContext);

      *outFrame = mxre::gl_utils::exportGLBufferToCV(width, height);
      return true;
    }


    /* Kernel Run */
    raft::kstatus ObjectRenderer::run()
    {
      if(!binding) {
        mxre::egl_utils::bindPbuffer(*pbuf);
        binding = true;
      }

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif
      debug_print("START");

      // 0.0.Get inputs from the previous kernel: ObjectDetector
      auto &inFrame( input["in_frame"].peek<mxre::types::Frame>() );
      auto &inObjContext( input["in_obj_context"].peek<std::vector<mxre::gl_types::ObjectContext>>() );

      // 0.1.Get input keystroke from Keyboard
      char inKey;
      if(checkPort("in_keystroke")) {
        inKey = input ["in_keystroke"].peek<char>();
        std::cout << "Input Key captured object_renderer: " << inKey << std::endl;
        recyclePort("in_keystroke");
      }
      else inKey = 0;

      // 0.2.Set output
      auto &outFrame( output["out_frame"].allocate<mxre::types::Frame>() );

      if(logic(&inFrame, &inObjContext, inKey, &outFrame)) {
        output["out_frame"].send();
        sendFrameCopy("out_frame", &outFrame);
      }

      recyclePort("in_frame");
      recyclePort("in_obj_context");

      debug_print("END");
#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }

  }   // namespace kernels
} // namespace mxre

