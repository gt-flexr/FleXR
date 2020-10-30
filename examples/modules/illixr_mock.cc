#include <bits/stdc++.h>
#include <mxre.h>
#include <unistd.h>

using namespace std;

int main() {
  mxre::types::ILLIXRSink<int> illixrSink;
  mxre::types::ILLIXRSource<int> illixrSource;

  illixrSource.setup("source");
  illixrSink.setup("sink");

  int j;
  for(int i = 0; i < 100; i++) {
    illixrSource.send(&i);
    illixrSink.recv(&j);
    printf("Send %d, Recv %d \n", i, j);
    sleep(1);
  }
}

