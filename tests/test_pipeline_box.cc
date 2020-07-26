#include <cstdio>
#include <iostream>
#include <sys/time.h>

#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/cudafeatures2d.hpp> // for cuda kp extractions
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include <EGL/egl.h>
#include <GL/glew.h>
#include <GL/glut.h>

#define WIDTH 1920
#define HEIGHT 1080
#define ITERATION 10

using namespace std;
using timmeval=struct timeval;

GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};  /* Red diffuse light. */
GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};  /* Infinite light location. */
GLfloat n[6][3] = {  /* Normals for the 6 faces of a cube. */
  {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
  {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0} };
GLint faces[6][4] = {  /* Vertex indices for the 6 faces of a cube. */
  {0, 1, 2, 3}, {3, 2, 6, 7}, {7, 6, 5, 4},
  {4, 5, 1, 0}, {5, 6, 2, 1}, {7, 4, 0, 3} };
GLfloat v[8][3];  /* Will be filled in with X,Y,Z vertexes. */


void drawBox(void) {
  for (int i = 0; i < 6; i++) {
    glBegin(GL_QUADS);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}

void
display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawBox();
  glFlush();
}



void init(void)
{
  /* Setup cube vertex data. */
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

  /* Enable a single OpenGL light. */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);

  /* Use depth buffering for hidden surface elimination. */
  glEnable(GL_DEPTH_TEST);

  /* Setup the view of the cube. */
  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 50.0,
    /* aspect ratio */ 1.0,
    /* Z near */ 1.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
            0.0, 0.0, 0.0,      /* center is at (0,0,0) */
            0.0, 1.0, 0.);      /* up is in positive Y direction */

  /* Adjust cube position to be asthetic angle. */
  glTranslatef(0.0, 0.0, -1.0);
  glRotatef(60, 1.0, 0.0, 0.0);
  glRotatef(-20, 0.0, 0.0, 1.0);
}

static const EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_BLUE_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_RED_SIZE, 8,
    EGL_DEPTH_SIZE, 8,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_NONE};

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, WIDTH,
    EGL_HEIGHT, HEIGHT,
    //EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
    //EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
    EGL_NONE,
};

cv::Mat capture()
{
  glReadBuffer(GL_FRONT);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  unsigned char *pixels = new unsigned char[3 * WIDTH * HEIGHT];
  glReadPixels(0, 0, WIDTH, HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, pixels);

  return cv::Mat(HEIGHT, WIDTH, CV_8UC3, pixels);
}

void cv2gl(cv::Mat) {

}


/*    Camera    */
class Camera: public raft::kernel {
  private:
  cv::VideoCapture cam;
  int frame_idx;

  public:
  Camera(): raft::kernel() {
    cam.open(2, cv::CAP_ANY);
    cam.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
    cam.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
    if( !cam.isOpened() ) {
      cerr << "ERROR: unable to open camera" << endl;
    }

    frame_idx = 0;
    output.addPort<cv::Mat>("o1");
    output.addPort<timeval>("o2");
  }

  virtual raft::kstatus run() {
    while(frame_idx++ < ITERATION) {
      auto frame(output["o1"].template allocate_s<cv::Mat>());
      auto time_stamp(output["o2"].template allocate_s<timeval>());

      cam.read(*frame);
      if((*frame).empty()) {
        cerr << "ERROR: blank frame grabbed" << endl;
        break;
      }

      gettimeofday(&(*time_stamp), NULL);

      output["o1"].send();
      output["o2"].send();
      return raft::proceed;
    }
    return raft::stop;
  }
};


/*    CLProcessor    */
class CLProcessor: public raft::kernel {
  private:
    cv::Ptr<cv::ORB> orb;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    cv::Mat descs;
    vector<cv::KeyPoint> kps;

  public:
  CLProcessor(): raft::kernel() {
    orb = cv::ORB::create();
    matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");

    input.addPort<cv::Mat>("i1");
    input.addPort<timeval>("i2");

    output.addPort<cv::Mat>("o1");
    output.addPort<timeval>("o2");
  }

  virtual raft::kstatus run() {
    cout << "CLProcessor START... ";
    auto frame = input["i1"].peek<cv::Mat>();
    cv::Mat gray_frame;
    auto st = input["i2"].peek<timeval>();

    // Do CL Work Here
    timeval lst, et;
    gettimeofday(&lst, NULL);

    // CPU version ORB
    cv::cvtColor(frame, gray_frame, CV_BGR2GRAY);
    orb->detectAndCompute(gray_frame, cv::noArray(), kps, descs);
    // GPU version ORB
    //   TODO...
    gettimeofday(&et, NULL);
    timersub(&et, &lst, &et);

    double exe_time = double(et.tv_sec) * 1000 + double(et.tv_usec) / 1000;
    cout << " END " << exe_time << "ms" << endl;

    output["o1"].template allocate_s<cv::Mat>(frame);
    output["o2"].template allocate_s<timeval>(st);
    output["o1"].send();
    output["o2"].send();

    input["i1"].recycle();
    input["i2"].recycle();
    return raft::proceed;
  }
};


/*    GLConsumer    */
class GLConsumer: public raft::kernel {
  private:
  EGLDisplay eglDpy;
  EGLSurface eglSurf;
  EGLContext eglCtx;
  EGLint major, minor, numConfigs;
  EGLConfig eglCfg;


  public:
  ~GLConsumer() {
    if(eglDpy) eglTerminate(eglDpy);
  }
  GLConsumer(): raft::kernel() {
    input.addPort<cv::Mat>("i1");
    input.addPort<timeval>("i2");

    // 1. get & init EGL Display
    eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(eglDpy, &major, &minor);

    // 2. config EGL display
    eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs);

    // 3. create surface with pbuffer
    eglSurf = eglCreatePbufferSurface(eglDpy, eglCfg, pbufferAttribs);

    // 4. bind OGL apis
    eglBindAPI(EGL_OPENGL_API);

    // 5. create context
    eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, NULL);
  }

  virtual raft::kstatus run() {
    // make the context current
    eglMakeCurrent(eglDpy, eglSurf, eglSurf, eglCtx);
    init();
    // init gl

    cout << "GLConsumer START... ";
    auto frame = input["i1"].peek<cv::Mat>();
    auto st = input["i2"].peek<timeval>();
    timeval lst, et, let;
    gettimeofday(&lst, NULL);

    // Do GL Work Here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawBox();
    glFlush();

    cv::Mat pixelData = capture(); // about 6ms
    gettimeofday(&et, NULL);
    timersub(&et, &lst, &let);
    timersub(&et, &st, &et);

    double consumer_exe_time = double(let.tv_sec) * 1000 + double(let.tv_usec) / 1000;
    cout << " END " << consumer_exe_time << "ms" << endl;
    double exe_time = double(et.tv_sec) * 1000 + double(et.tv_usec) / 1000;
    cout << "TOTAL: " << exe_time << "ms" << endl << endl;

    cv::imwrite("result.jpg", pixelData);

    input["i1"].recycle();
    input["i2"].recycle();

    return raft::proceed;
  }
};


int main(int argc, char **argv) {
  Camera cam;
  CLProcessor clp;
  GLConsumer glc;

  raft::map m;

  m += cam["o1"] >> clp["i1"];
  m += cam["o2"] >> clp["i2"];

  m += clp["o1"] >> glc["i1"];
  m += clp["o2"] >> glc["i2"];

  m.exe();

  return EXIT_SUCCESS;
}