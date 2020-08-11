#include <include/object_renderer.h>
#include <include/gl_objects.h>
namespace mxre
{
  namespace pipeline
  {
    namespace rendering
    {
      ObjectRenderer::ObjectRenderer() : raft::kernel()
      {
        mxre::eglutils::initEGLPbuffer(pbuf);

        input.addPort<cv::Mat>("in_frame");
        input.addPort<std::vector<mxre::gltypes::ObjectContext>>("in_obj_context");
        input.addPort<clock_t>("in_timestamp");

        output.addPort<cv::Mat>("out_frame");
        output.addPort<clock_t>("out_timestamp");
      }

      ObjectRenderer::~ObjectRenderer() {
        mxre::eglutils::terminatePbuffer(pbuf);
      }

      raft::kstatus ObjectRenderer::run()
      {
        clock_t rt = clock();
        mxre::eglutils::setCurrentPbuffer(pbuf);
        mxre::glutils::initGL(WIDTH, HEIGHT);

        // get inputs from the previous kernel: ObjectDetector
        auto frame = input["in_frame"].peek<cv::Mat>();
        auto objCtxVec = input["in_obj_context"].peek<std::vector<mxre::gltypes::ObjectContext>>();
        auto in_st = input["in_timestamp"].peek<clock_t>();

        printf("[ObjectRenderer] ObjCtxExtractor->ObjRenderer communication cost %f ms \n",
               ((float)(rt)-in_st) / CLOCKS_PER_SEC * 1000);

        // set outputs
        auto out_frame = output["out_frame"].template allocate_s<cv::Mat>();
        auto out_ts = output["out_timestamp"].template allocate_s<clock_t>();

        // 1. Create/update background texture
        if(glIsTexture(backgroundTexture))
          mxre::glutils::updateTextureFromCVFrame(frame, backgroundTexture);
        else
          mxre::glutils::makeTextureFromCVFrame(frame, backgroundTexture);

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
          //printf("Translation info [obj renderer]: %f %f %f \n", objCtxIter->transVec.x, objCtxIter->transVec.y, objCtxIter->transVec.z);
          //objCtxIter->rotMat.translate(objCtxIter->transVec.x, objCtxIter->transVec.y, objCtxIter->transVec.z);
          printf("ObjCtxIter->modelMat=================== \n");
          printf("%f %f %f %f\n %f %f %f %f\n %f %f %f %f \n %f %f %f %f \n",
           objCtxIter->modelMat[0], objCtxIter->modelMat[1], objCtxIter->modelMat[2], objCtxIter->modelMat[3],
           objCtxIter->modelMat[4], objCtxIter->modelMat[5], objCtxIter->modelMat[6], objCtxIter->modelMat[7],
           objCtxIter->modelMat[8], objCtxIter->modelMat[9], objCtxIter->modelMat[10], objCtxIter->modelMat[11],
           objCtxIter->modelMat[12], objCtxIter->modelMat[13], objCtxIter->modelMat[14], objCtxIter->modelMat[15]
          );
          printf("===================================== \n");
          glLoadMatrixf(objCtxIter->modelMat.get());

          if(objCtxIter->index == 0) mxre::globjs::drawTeapot();
          else mxre::globjs::drawBox();
          //mxre::globjs::drawBox();
          glFlush();
        }

        glPopMatrix();
        cv::Mat resFrame = mxre::glutils::exportGLBufferToCV();
        *out_frame = resFrame;
        *out_ts = clock();

        input["in_frame"].recycle();
        input["in_obj_context"].recycle();
        input["in_timestamp"].recycle();

        printf("[ObjectCtxDetector] exe time %f ms \n", ((float)(*out_ts) - in_st) / CLOCKS_PER_SEC * 1000);

        output["out_frame"].send();
        output["out_timestamp"].send();
        return raft::proceed;
      }

    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre