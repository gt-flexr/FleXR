#include <raft>
#include <mxre>
#include <bits/stdc++.h>

using namespace std;


int main(int argc, char const *argv[])
{
  raft::map testPipeline;

  mxre::kernels::CVMatAppSource appsource("source");
  mxre::kernels::CVMatAppSink appsink("sink");

  testPipeline += appsource["out_data"] >> appsink["in_data"];
  testPipeline.exe();

  return 0;
}

