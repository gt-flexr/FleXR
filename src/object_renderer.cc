#include <include/object_renderer.h>
#include <include/gl_objects.h>
#include <include/cv_types.h>

namespace mxre
{
  namespace pipeline
  {
    namespace rendering
    {
      ObjectRenderer::ObjectRenderer() : raft::kernel()
      {
        mxre::eglutils::initEGLPbuffer(pbuf);

        input.addPort<mxre::cv_units::Mat>("in_frame");
        input.addPort<std::vector<mxre::gltypes::ObjectContext>>("in_obj_context");

        output.addPort<mxre::cv_units::Mat>("out_frame");
      }

      ObjectRenderer::~ObjectRenderer() {
        mxre::eglutils::terminatePbuffer(pbuf);
      }

      raft::kstatus ObjectRenderer::run()
      {

#ifdef __PROFILE__
        TimeVal start = getNow();
#endif

        mxre::eglutils::setCurrentPbuffer(pbuf);
        mxre::glutils::initGL(WIDTH, HEIGHT);

        // get inputs from the previous kernel: ObjectDetector
        auto &frame( input["in_frame"].peek<mxre::cv_units::Mat>() );
        auto objCtxVec( input["in_obj_context"].peek<std::vector<mxre::gltypes::ObjectContext>>() );

        // set outputs
        auto &out_frame( output["out_frame"].allocate<mxre::cv_units::Mat>() );

        // 1. Create/update background texture & release previous CV frame
        if(glIsTexture(backgroundTexture)) {
          mxre::glutils::updateTextureFromCVFrame(frame, backgroundTexture);
        }
        else {
          mxre::glutils::makeTextureFromCVFrame(frame, backgroundTexture);
          mxre::glutils::updateTextureFromCVFrame(frame, backgroundTexture);
        }
        frame.release();

        // 2. Draw background frame
        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();

        mxre::glutils::startBackground(WIDTH, HEIGHT);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glBegin(GL_QUADS);
        glColor3f(1, 1, 1);
          glTexCoord2i(0,0); glVertex3f(0, 0, -1);
          glTexCoord2i(1,0); glVertex3f(WIDTH, 0, -1);
          glTexCoord2i(1,1); glVertex3f(WIDTH, HEIGHT, -1);
          glTexCoord2i(0,1); glVertex3f(0, HEIGHT, -1);
        glEnd();
        mxre::glutils::endBackground();

        std::vector<mxre::gltypes::ObjectContext>::iterator objCtxIter;
        for (objCtxIter = objCtxVec.begin(); objCtxIter != objCtxVec.end(); ++objCtxIter)
        {
          /*
          printf("Translation info [obj renderer]: %f %f %f \n", objCtxIter->transVec.x, objCtxIter->transVec.y,
                                                                 objCtxIter->transVec.z);
          objCtxIter->rotMat.translate(objCtxIter->transVec.x, objCtxIter->transVec.y, objCtxIter->transVec.z);
          printf("ObjCtxIter->modelMat=================== \n");
          printf("%f %f %f %f\n %f %f %f %f\n %f %f %f %f \n %f %f %f %f \n",
           objCtxIter->modelMat[0], objCtxIter->modelMat[1], objCtxIter->modelMat[2], objCtxIter->modelMat[3],
           objCtxIter->modelMat[4], objCtxIter->modelMat[5], objCtxIter->modelMat[6], objCtxIter->modelMat[7],
           objCtxIter->modelMat[8], objCtxIter->modelMat[9], objCtxIter->modelMat[10], objCtxIter->modelMat[11],
           objCtxIter->modelMat[12], objCtxIter->modelMat[13], objCtxIter->modelMat[14], objCtxIter->modelMat[15]
          );
          printf("===================================== \n");
          */
          glLoadMatrixf(objCtxIter->modelMat.get());

          if(objCtxIter->index == 1) mxre::globjs::drawTeapot();
          else mxre::globjs::drawBox();
          //mxre::globjs::drawBox();
          glFlush();
        }
        glPopMatrix();
        out_frame = mxre::glutils::exportGLBufferToCV();

        input["in_frame"].recycle();
        input["in_obj_context"].recycle();

#ifdef __PROFILE__
        TimeVal end = getNow();
        debug_print("Exe Time: %lfms", getExeTime(end, start));
#endif

        output["out_frame"].send();
        return raft::proceed;
      }

    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre

