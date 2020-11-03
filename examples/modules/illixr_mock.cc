#include <bits/stdc++.h>
#include <mxre>
#include <unistd.h>

using namespace std;

int main() {
  mxre::types::ILLIXRSource<int> illixrSource;
  mxre::types::ILLIXRSink<int> illixrSink;

  illixrSource.setup("source", MX_DTYPE_PRIMITIVE);
  illixrSink.setup("sink", MX_DTYPE_PRIMITIVE);

  int j;
  for(int i = 0; i < 100; i++) {
    illixrSource.send(&i);
    illixrSink.recv(&j);
    printf("Send %d, Recv %d \n", i, j);
    sleep(1);
  }
}

