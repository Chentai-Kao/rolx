#include "stdafx.h"
#include "egonet.h"
#include "rolx.h"

void PrintRole(PUNGraph Graph, TFltVV& G) {
  char* roleToColor[10];
  roleToColor[0] = "white";
  roleToColor[1] = "black";
  roleToColor[2] = "red";
  roleToColor[3] = "green";
  roleToColor[4] = "blue";
  roleToColor[5] = "yellow";
  roleToColor[6] = "gold";
  roleToColor[7] = "cyan";
  roleToColor[8] = "magenta";
  roleToColor[9] = "brown";
  TIntStrH color;
  TIntV roles;
  for (int i = 0; i < G.GetXDim(); i++) {
    float max = 0; 
    int role;
    for (int j = 0; j < G.GetYDim(); j ++) {
      if (G.At(i, j) > max) {
        max = G.At(i, j);
        role = j;
      }
    }
    roles.Add(role);
  }
  for (int i = 0; i < roles.Len(); i++) {
    color.AddDat(i) = roleToColor[roles.GetVal(i)];
  }
  TSnap::DrawGViz(Graph, gvlDot, "gviz_plot.png", "Dot", 1, color);
  //TGraphViz::Plot<PNGraph>(Graph, gvlDot, "gviz_plot.png", "", color);
}

int main(int argc, char* argv[]) {
  PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(
      "dataset/Email-Enron.txt", 0, 1);

  TIntFtrH Features = ExtractFeatures(Graph);
  printf("finish feature extraction\n");
  //PrintFeatures(Features);
  TFltVV V = ConvertFeatureToMatrix(Features); 
  //PrintMatrix(V);

  TFlt MnError = TFlt::Mx;
  TFltVV FinalG, FinalF;
  int NumRoles = -1;
  for (int r = 3; r < 4; ++r) {
    TFltVV G, F;
    CalcNonNegativeFactorization(V, r, G, F);
    printf("finish factorization\n");
    //PrintMatrix(G);
    TFlt Error = ComputeDescriptionLength(V, G, F);
    if (Error < MnError) {
      MnError = Error;
      FinalG = G;
      FinalF = F;
      NumRoles = r;
    }
  }
  //printf("--FinalG--\n");
  PrintMatrix(FinalG);
  //printf("--FinalF--\n");
  //PrintMatrix(FinalF);
  printf("using %d roles\n", NumRoles);

  PrintRole(Graph, FinalG);

  return 0;
}
