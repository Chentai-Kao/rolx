#include "stdafx.h"
#include "egonet.h"
#include "rolx.h"

int main(int argc, char* argv[]) {
  PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(
      "dataset/facebook_combined_small.txt", 0, 1);

  TIntFtrH Features = ExtractFeatures(Graph);

  return 0;
}
