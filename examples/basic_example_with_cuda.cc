#ifdef __USE_OPENCV_CUDA__
#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;

int main(int argc, char const *argv[])
{
  cv::CommandLineParser parser(argc, argv,
      "{@input_path |0|input path can be a camera id, like 0,1,2 or a video filename}");
  parser.printMessage();
  string input_path = parser.get<string>(0);
  string video_name = input_path;
  int camera_no;

  cv::VideoCapture video_src;
  if ((isdigit(input_path[0]) && input_path.size() == 1))
  {
    camera_no = input_path[0] - '0';
    video_src.open(camera_no, cv::CAP_ANY);
    video_src.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
    video_src.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
  }
  else
  {
    video_src.open(video_name);
  }

  if (!video_src.isOpened())
  {
    cerr << "Couldn't open " << video_name << endl;
    return 1;
  }

  cv::Ptr<cv::ORB> orb = cv::ORB::create();
  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker(orb, matcher);

  cv::Mat frame;
  cv::namedWindow(video_name, cv::WINDOW_NORMAL);

  for (int i = 0; i < MAX_OBJS; i++)
  {
    cout << "\n" << i+1 << "th object selection (q to quit, c to select a bounding box)\n";
    int inKey;
    while (true)
    {
      inKey = cv::waitKey(1) & 0xFF;
      if (inKey == 'q')
      {
        i = MAX_OBJS;
        break;
      }
      if (inKey == 'c')
        break;

      video_src >> frame;
      cv::resizeWindow(video_name, frame.size());
      cv::imshow(video_name, frame);
    }

    if (inKey == 'c')
    {
      vector<cv::Point2f> roi;
      cv::Rect roiRect = cv::selectROI(video_name, frame);
      if(roiRect.width == 0 || roiRect.height == 0)
        i--;
      else
        orbMarkerTracker.registerObject(frame, roiRect);
    }
  }
  orbMarkerTracker.printRegisteredObjects();
  video_src.release();

  mxre::kernels::CVCamera cam(camera_no, WIDTH, HEIGHT);
  mxre::kernels::Keyboard keyboard;
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::ObjectCtxExtractor objCtxExtractor(cam.getIntrinsic(), cam.getDistCoeffs(),
      WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::CVDisplay cvDisplay;

  raft::map pipeline;

  // cam - obj detector
  pipeline += cam["out_frame"] >> cudaORBDetector["in_frame"];

  // obj detector - obj ctx extractor
  pipeline += cudaORBDetector["out_frame"] >> objCtxExtractor["in_frame"];
  pipeline += cudaORBDetector["out_obj_info"] >> objCtxExtractor["in_obj_info"];

  // obj ctx extractor - obj renderer
  pipeline += objCtxExtractor["out_frame"] >> objRenderer["in_frame"];
  pipeline += objCtxExtractor["out_obj_context"] >> objRenderer["in_obj_context"];
  pipeline += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];

  // obj renderer - test sink
  pipeline += objRenderer["out_frame"] >> cvDisplay["in_frame"];

#ifdef __PROFILE__
  pipeline += cam["frame_stamp"] >> cudaORBDetector["frame_stamp"];
  pipeline += cudaORBDetector["frame_stamp"] >> objCtxExtractor["frame_stamp"];
  pipeline += objCtxExtractor["frame_stamp"] >> objRenderer["frame_stamp"];
  pipeline += objRenderer["frame_stamp"] >> cvDisplay["frame_stamp"];
#endif

  pipeline.exe();
  return 0;
}

#endif

