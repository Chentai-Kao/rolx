#include <string.h>
#include "stdafx.h"
#include "egonet.h"
#include "rolx.h"

void PrintRole(const PUNGraph Graph, const TIntIntH& Roles) {
  std::string RoleToColor[10] = { "white", "black", "red", "green", "blue",
      "yellow", "gold", "cyan", "magenta", "brown" };
  TIntStrH Color;
  for (TIntIntH::TIter HI = Roles.BegI(); HI < Roles.EndI(); HI++) {
    Color.AddDat(HI.GetKey(), RoleToColor[HI.GetDat()].c_str());
  }
  TSnap::DrawGViz(Graph, gvlDot, "gviz_plot.png", "Dot", 1, Color);
  //TGraphViz::Plot<PNGraph>(Graph, gvlDot, "gviz_plot.png", "", Color);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: ./prototype <dataset file>\n");
    exit(EXIT_SUCCESS);
  }

  PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(
      argv[1], 0, 1);
  printf("finish loading file\n");

  TIntFtrH Features = ExtractFeatures(Graph);
  printf("finish feature extraction\n");
  printf("--features--\n");
  PrintFeatures(Features);
  TIntIntH NodeIdMtxIdH = CreateNodeIdMtxIdxHash(Features);
  printf("--finish create (node ID -> Mtx ID) hash--\n");
  TFltVV V = ConvertFeatureToMatrix(Features, NodeIdMtxIdH); 
  printf("--finish convert feature to matrix--\n");
  //printf("--feature matrix--\n");
  //PrintMatrix(V);

  TFlt MnError = TFlt::Mx;
  TFltVV FinalG, FinalF;
  int NumRoles = -1;
  for (int r = 3; r < 4; ++r) {
    TFltVV G, F;
    CalcNonNegativeFactorization(V, r, G, F);
    printf("finish factorization for r=%d\n", r);
    //PrintMatrix(G);
    TFlt Error = ComputeDescriptionLength(V, G, F);
    if (Error < MnError) {
      MnError = Error;
      FinalG = G;
      FinalF = F;
      NumRoles = r;
    }
  }
  printf("--FinalG--\n");
  PrintMatrix(FinalG);
  printf("--FinalF--\n");
  PrintMatrix(FinalF);
  printf("using %d roles, min error: %f\n", NumRoles, MnError());

  TIntIntH Roles = FindRoles(FinalG, NodeIdMtxIdH);
  PrintRole(Graph, Roles);

  return 0;
}
