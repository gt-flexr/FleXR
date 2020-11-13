#include <raft>
#include <mxre>
#include <bits/stdc++.h>

using namespace std;


int main(int argc, char const *argv[])
{
  raft::map testPipeline;

  mxre::kernels::AppSource<mxre::types::Frame> appsource;
  mxre::kernels::AppSink<mxre::types::Frame> appsink;

  appsource.setup("source", MXRE_DTYPE_FRAME);
  appsink.setup("sink", MXRE_DTYPE_FRAME);

  testPipeline += appsource["out_data"] >> appsink["in_data"];
  testPipeline.exe();

  return 0;
}

