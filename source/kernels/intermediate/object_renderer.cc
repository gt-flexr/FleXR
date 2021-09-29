#include <kernels/intermediate/object_renderer.h>
#include <GL/gl.h>
#include <GL/glew.h>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {

    ObjectRenderer::ObjectRenderer(std::string id, int width, int height) :
      FleXRKernel(id), width(width), height(height)
    {
      setName("ObjectRenderer");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
      portManager.registerInPortTag("in_marker_contexts",
                                    components::PortDependency::NONBLOCKING);
      portManager.registerInPortTag("in_key",
                                    components::PortDependency::NONBLOCKING);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy);

      // 0. Create pbuf as a context
      pbuf = new flexr::egl_types::pbuffer;
      flexr::egl_utils::initPbuffer(*pbuf, width, height);

      // 1. Init pbuf as context and GL with the context
      flexr::egl_utils::bindPbuffer(*pbuf);
      flexr::gl_utils::initGL(width, height);

      // 2. Set AR Worlds via WorldManager
      // 2.1. Init shader
      worldManager.initShader();

      // 2.2. Add new worlds
      //std::vector<flexr::cv_types::MarkerInfo>::iterator markerInfo;
      //for(markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo) {
      //  worldManager.addWorld(markerInfo->index);
      //}

      worldManager.addWorld(0);
      // 2.3. Add an object to each world (temporarily)
      for(int i = 0; i < worldManager.numOfWorlds; i++)
        worldManager.addObjectToWorld(i);

      // 3. Unbind the pbuf context in init thread
      flexr::egl_utils::unbindPbuffer(*pbuf);
      binding = false;
    }


    ObjectRenderer::~ObjectRenderer()
    {
      flexr::egl_utils::terminatePbuffer(*pbuf);
      delete [] pbuf;
    }


    bool ObjectRenderer::logic(ObjRendererInFrameType  *inFrame,
                               ObjRendererInKeyType    *inKey,
                               ObjRendererInCtxType    *inMarkerContexts,
                               ObjRendererOutFrameType *outFrame)
    {
      char key = 0;
      if(inKey != nullptr) key = inKey->data;

      if(glIsTexture(backgroundTexture)) {
        flexr::gl_utils::updateTextureFromFrame(&inFrame->data, backgroundTexture);
      }
      else {
        flexr::gl_utils::makeTextureFromFrame(&inFrame->data, backgroundTexture);
      }

      // 2. Draw background frame
      glClearColor(1.0, 1.0, 1.0, 1.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      flexr::gl_utils::startBackground(width, height);
      glBindTexture(GL_TEXTURE_2D, backgroundTexture);
      glBegin(GL_QUADS);
      glColor3f(1, 1, 1);
        glTexCoord2i(0,0); glVertex3f(0,     0,      -1);
        glTexCoord2i(1,0); glVertex3f(width, 0,      -1);
        glTexCoord2i(1,1); glVertex3f(width, height, -1);
        glTexCoord2i(0,1); glVertex3f(0,     height, -1);
      glEnd();
      flexr::gl_utils::endBackground();

      if(inMarkerContexts != nullptr)
      {
        if(inMarkerContexts->data.size() > 0) {
          worldManager.startWorlds(key, inMarkerContexts->data);

          outFrame->data = flexr::gl_utils::exportGLBufferToCV(width, height);
          strcpy(outFrame->tag, inMarkerContexts->tag);
          outFrame->ts = inMarkerContexts->ts;
          outFrame->seq = inMarkerContexts->seq;
          return true;
        }
      }

      outFrame->data = flexr::gl_utils::exportGLBufferToCV(width, height);
      strcpy(outFrame->tag, inFrame->tag);
      outFrame->ts = inFrame->ts;
      outFrame->seq = inFrame->seq;
      return true;
    }


    raft::kstatus ObjectRenderer::run()
    {
      if(!binding) {
        flexr::egl_utils::bindPbuffer(*pbuf);
        binding = true;
      }

      ObjRendererInCtxType    *inMarkerContexts = portManager.getInput<ObjRendererInCtxType>("in_marker_contexts");
      ObjRendererInFrameType  *inFrame  = portManager.getInput<ObjRendererInFrameType>("in_frame");
      ObjRendererInKeyType    *inKey    = portManager.getInput<ObjRendererInKeyType>("in_key");
      ObjRendererOutFrameType *outFrame = portManager.getOutputPlaceholder<ObjRendererOutFrameType>("out_frame");

      double st = getTsNow();
      logic(inFrame, inKey, inMarkerContexts, outFrame);
      portManager.sendOutput("out_frame", outFrame);
      double et = getTsNow();

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);
      portManager.freeInput("in_marker_contexts", inMarkerContexts);
      portManager.freeInput("in_key", inKey);

      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);

      return raft::proceed;
    }

  }   // namespace kernels
} // namespace flexr

