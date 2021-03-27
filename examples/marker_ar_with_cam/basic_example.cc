#include <raft>
#include <mxre>
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
  //string video_name = input_path;
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
    video_src.open(input_path);
  }

  if (!video_src.isOpened())
  {
    cerr << "Couldn't open " << input_path << endl;
    return 1;
  }

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;

  cv::Mat frame;
  cv::namedWindow(input_path, cv::WINDOW_NORMAL);

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
      cv::resizeWindow(input_path, frame.size());
      cv::imshow(input_path, frame);
    }

    if (inKey == 'c')
    {
      vector<cv::Point2f> roi;
      cv::Rect roiRect = cv::selectROI(input_path, frame);
      if(roiRect.width == 0 || roiRect.height == 0)
        i--;
      else
        orbMarkerTracker.registerObject(frame, roiRect);
    }
  }
  orbMarkerTracker.printRegisteredObjects();
  video_src.release();

  mxre::kernels::CVCamera cam(camera_no, WIDTH, HEIGHT);
  cam.duplicateOutPort<mxre::types::Frame>("out_frame", "out_frame2");
  mxre::kernels::Keyboard keyboard;
  mxre::kernels::ORBDetector orbDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::MarkerCtxExtractor markerCtxExtractor(WIDTH, HEIGHT, cam.getIntrinsic(), cam.getDistCoeffs());
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::CVDisplay cvDisplay;

  raft::map pipeline;

  // cam - obj detector
  pipeline += cam["out_frame"] >> orbDetector["in_frame"];

  // obj detector - obj ctx extractor
  pipeline += orbDetector["out_detected_markers"] >> markerCtxExtractor["in_detected_markers"];


  // obj ctx extractor - obj renderer
  pipeline += cam["out_frame2"] >> objRenderer["in_frame"];
  pipeline += markerCtxExtractor["out_marker_contexts"] >> objRenderer["in_marker_contexts"];
  pipeline += keyboard["out_keystroke"] >> objRenderer["in_keystroke"];

  // obj renderer - test sink
  pipeline += objRenderer["out_frame"] >> cvDisplay["in_frame"];

  cout << "pipeline ext()" << endl;
  pipeline.exe();
  return 0;
}

