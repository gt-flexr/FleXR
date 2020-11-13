#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;


int main(int argc, char const *argv[])
{
  /* set markers */
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

  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;

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

  cv::Mat defaultDistCoeffs = cv::Mat(4, 1, CV_64FC1, {0, 0, 0, 0});
  cv::Mat defaultIntrinsic = cv::Mat(3, 3, CV_64FC1);
  defaultIntrinsic.at<double>(0, 0) = WIDTH; defaultIntrinsic.at<double>(0, 1) = 0; defaultIntrinsic.at<double>(0, 2) = WIDTH/2;
  defaultIntrinsic.at<double>(1, 0) = 0; defaultIntrinsic.at<double>(1, 1) = WIDTH; defaultIntrinsic.at<double>(1, 2) = HEIGHT/2;
  defaultIntrinsic.at<double>(2, 0) = 0; defaultIntrinsic.at<double>(2, 1) = 0; defaultIntrinsic.at<double>(2, 2) = 1;

  raft::map servingPipeline;
  mxre::kernels::CudaORBDetector cudaORBDetector(orbMarkerTracker.getRegisteredObjects());
  mxre::kernels::ObjectCtxExtractor objCtxExtractor(defaultIntrinsic, defaultDistCoeffs, WIDTH, HEIGHT);
  mxre::kernels::ObjectRenderer objRenderer(orbMarkerTracker.getRegisteredObjects(), WIDTH, HEIGHT);
  mxre::kernels::RTPFrameReceiver rtpReceiver("mjpeg", 49985, WIDTH, HEIGHT);
  rtpReceiver.duplicateOutPort<mxre::types::Frame>("out_data", "out_data2");
  mxre::kernels::StaticReceiver<char> keyReceiver(49986, false);
  mxre::kernels::RTPFrameSender rtpSender("mjpeg", "127.0.0.1", 49987, 800000, 10, WIDTH, HEIGHT);

  //mxre::pipeline::output_sinks::CVDisplay cvDisplay;

  servingPipeline += rtpReceiver["out_data"] >> cudaORBDetector["in_frame"];

  servingPipeline += cudaORBDetector["out_obj_info"] >> objCtxExtractor["in_obj_info"];

  servingPipeline += rtpReceiver["out_data2"] >> objRenderer["in_frame"];
  servingPipeline += objCtxExtractor["out_obj_context"] >> objRenderer["in_obj_context"];
  servingPipeline += keyReceiver["out_data"] >> objRenderer["in_keystroke"];

  servingPipeline += objRenderer["out_frame"] >> rtpSender["in_data"];
  servingPipeline.exe();
  return 0;
}

