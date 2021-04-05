#include <raft>
#include <mxre>
#include <bits/stdc++.h>
using namespace std;


int main(int argc, char const *argv[])
{
  raft::map testPipeline;

  mxre::kernels::AppSource<int> appsource;
  mxre::kernels::AppSink<int> appsink;

  appsource.setup("source");
  appsink.setup("sink");

  testPipeline += appsource["out_data"] >> appsink["in_data"];
  testPipeline.exe();

  return 0;
}

