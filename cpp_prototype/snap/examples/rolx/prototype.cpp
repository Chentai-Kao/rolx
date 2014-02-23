#include "stdafx.h"
#include "egonet.h"
#include "rolx.h"

int main(int argc, char* argv[]) {
  PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(
      "dataset/Email-Enron.txt", 0, 1);

  TIntFtrH Features = ExtractFeatures(Graph);
  printf("finish feature extraction\n");
  TFltVV V = ConvertFeatureToMatrix(Features);

  TFlt MnError = TFlt::Mx;
  TFltVV FinalG, FinalF;
  int NumRoles = -1;
  for (int r = 1; r < 2; ++r) {
    TFltVV G, F;
    CalcNonNegativeFactorization(V, r, G, F);
    printf("finish factorization\n");
    TFlt Error = ComputeDescriptionLength(V, G, F);
    if (Error < MnError) {
      MnError = Error;
      FinalG = G;
      FinalF = F;
      NumRoles = r;
    }
  }
  //printf("--FinalG--\n");
  //PrintMatrix(FinalG);
  //printf("--FinalF--\n");
  //PrintMatrix(FinalF);
  printf("using %d roles\n", NumRoles);

  return 0;
}
