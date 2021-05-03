#include <kernels/intermediate/object_renderer.h>
#include <GL/gl.h>
#include <GL/glew.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {

    /* Constructor */
    ObjectRenderer::ObjectRenderer(std::vector<mxre::cv_types::MarkerInfo> registeredMarkers, int width, int height) :
      MXREKernel(), width(width), height(height)
    {
      addInputPort<types::Message<types::Frame>>("in_frame");
      addInputPort<types::Message<std::vector<gl_types::ObjectContext>>>("in_marker_contexts");
      addInputPort<types::Message<char>>("in_key");
      addOutputPort<types::Message<types::Frame>>("out_frame");

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
      std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
      for(markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo) {
        worldManager.addWorld(markerInfo->index);
      }
      // 2.3. Add an object to each world (temporarily)
      for(int i = 0; i < worldManager.numOfWorlds; i++)
        worldManager.addObjectToWorld(i);

      // 3. Unbind the pbuf context in init thread
      mxre::egl_utils::unbindPbuffer(*pbuf);
      binding = false;

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("object_renderer", "logs/" + std::to_string(pid) + "/object_renderer.log");
#endif
    }


    /* Destructor */
    ObjectRenderer::~ObjectRenderer() {
      mxre::egl_utils::terminatePbuffer(*pbuf);
      delete [] pbuf;
    }


    /* Kernel Logic */
    bool ObjectRenderer::logic(types::Message<types::Frame> &inFrame,
                               types::Message<std::vector<gl_types::ObjectContext>> &inMarkerContexts,
                               char inKey,
                               types::Message<types::Frame> &outFrame)
    {
      // 1. Create/update background texture & release previous CV frame
      if(glIsTexture(backgroundTexture)) {
        mxre::gl_utils::updateTextureFromFrame(&inFrame.data, backgroundTexture);
      }
      else {
        mxre::gl_utils::makeTextureFromFrame(&inFrame.data, backgroundTexture);
      }
      inFrame.data.release();

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

      worldManager.startWorlds(inKey, inMarkerContexts.data);

      outFrame.data = mxre::gl_utils::exportGLBufferToCV(width, height, 0, 0);
      outFrame.tag = inMarkerContexts.tag;
      outFrame.seq = inMarkerContexts.seq;
      outFrame.ts  = inMarkerContexts.ts;
      return true;
    }


    /* Kernel Run */
    raft::kstatus ObjectRenderer::run()
    {
      if(!binding) {
        mxre::egl_utils::bindPbuffer(*pbuf);
        binding = true;
      }

      // 0.0.Get inputs from the previous kernel: ObjectDetector
      types::Message<types::Frame> &inFrame = input["in_frame"].peek<types::Message<types::Frame>>();
      types::Message<std::vector<gl_types::ObjectContext>> &inMarkerContexts = \
                              input["in_marker_contexts"].peek<types::Message<std::vector<gl_types::ObjectContext>>>();
      types::Message<types::Frame> &outFrame = output["out_frame"].allocate<types::Message<types::Frame>>();
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      // 0.1.Get input keystroke from Keyboard
      char key;
      if(checkPort("in_key")) {
        types::Message<char> inKey = input ["in_key"].peek<types::Message<char>>();
        std::cout << "Input Key captured object_renderer: " << inKey.data << std::endl;
        key = inKey.data;
        recyclePort("in_key");
      }
      else key = 0;

      // 0.2.Set output

      if(logic(inFrame, inMarkerContexts, key, outFrame)) {
        sendFrames("out_frame", outFrame);
      }

      recyclePort("in_frame");
      recyclePort("in_marker_contexts");

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif
      return raft::proceed;
    }

  }   // namespace kernels
} // namespace mxre

