#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

using namespace std;

class TestSink : public raft::kernel {
  public:
  TestSink(): raft::kernel() {
    input.addPort<cv::Mat>("in_frame");
    input.addPort<std::vector<mxre::cv_units::ObjectInfo>>("in_obj_info");
    input.addPort<clock_t>("in_timestamp");
  }

  virtual raft::kstatus run() {
    auto frame = input["in_frame"].peek<cv::Mat>();
    auto objInfoVec = input["in_obj_info"].peek<std::vector<mxre::cv_units::ObjectInfo>>();
    auto st = input["in_timestamp"].peek<clock_t>();

    cv::imshow("TestSink Received", frame);
    int inKey = cv::waitKey(100) & 0xFF;
    printf("[TestSink] st(%ld), objectDetected(%d) \n", st, objInfoVec[0].homography.empty());

    input["in_frame"].recycle();
    input["in_obj_info"].recycle();
    input["in_timestamp"].recycle();

    return raft::proceed;
  }
};

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
  mxre::cv_units::ObjectTracker objTracker(orb, matcher);

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
       objTracker.registerObject(frame, roiRect);
    }
  }
  objTracker.printRegisteredObjects();
  video_src.release();

  mxre::pipeline::input_srcs::Camera cam(camera_no);
  mxre::pipeline::ctx_understanding::ObjectDetector objDetector(objTracker.getRegisteredObjects(), orb, matcher);
  TestSink sink;
  //mxre::pipeline::contextualizing::ObjectCtxExtractor objCtxExtractor;
  //mxre::pipeline::rendering::ObjectOverlayer objOverlayer;
  //mxre::pipeline::output_sink::Display disp;

  raft::map pipeline;

  // cam - obj detector
  pipeline += cam["out_frame"] >> objDetector["in_frame"];
  pipeline += cam["out_timestamp"] >> objDetector["in_timestamp"];

  // obj detector - sink
  pipeline += objDetector["out_frame"] >> sink["in_frame"];
  pipeline += objDetector["out_timestamp"] >> sink["in_timestamp"];
  pipeline += objDetector["out_obj_info"] >> sink["in_obj_info"];

  pipeline.exe();
 return 0;
}
