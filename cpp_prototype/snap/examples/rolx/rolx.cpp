#include "rolx.h"

void PrintFeatures(const TIntFtrH& Features) {
  for (TIntFtrH::TIter HI = Features.BegI(); HI < Features.EndI(); HI++) {
    printf("%d: [", HI.GetKey()());
    const TFtr& f = HI.GetDat();
    for (int i = 0; i < f.Len(); ++i) {
      if (i > 0) {
        printf(",");
      }
      printf("%f", f[i]());
    }
    printf("]\n");
  }
}

TIntFtrH CreateEmptyFeatures(const PUNGraph Graph) {
  TIntFtrH EmptyFeatures;
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    EmptyFeatures.AddDat(TInt(NI.GetId()), TFtr());
  }
  return EmptyFeatures;
}

TIntFtrH CreateEmptyFeatures(const TIntFtrH& Features) {
  TIntFtrH EmptyFeatures;
  for (TIntFtrH::TIter HI = Features.BegI(); HI < Features.EndI(); HI++) {
    EmptyFeatures.AddDat(HI.GetKey(), TFtr());
  }
  return EmptyFeatures;
}

int GetNumFeatures(const TIntFtrH& Features) {
  return Features.BegI().GetDat().Len();
}

TFtr GetNthFeature(const TIntFtrH& Features, const int n) {
  TFtr NthFeature;
  IAssert(0 <= n && n < GetNumFeatures(Features));
  for (TIntFtrH::TIter HI = Features.BegI(); HI < Features.EndI(); HI++) {
    NthFeature.Add(HI.GetDat()[n]);
  }
  return NthFeature;
}

TIntFtrH ExtractFeatures(const PUNGraph Graph) {
  TIntFtrH Features = CreateEmptyFeatures(Graph);
  printf("finish createEmptyFeatures()\n");
  AddNeighborhoodFeatures(Graph, Features);
  printf("finish neighborhood features\n");
  AddRecursiveFeatures(Graph, Features);
  printf("finish recursive features\n");
  return Features;
}

void AddNeighborhoodFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  AddLocalFeatures(Graph, Features);
  printf("finish recursive features\n");
  AddEgonetFeatures(Graph, Features);
  printf("finish egonet features\n");
}

void AddRecursiveFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  int SimilarityThreshold = 0;
  TIntFtrH RetainedFeatures = Features;
  while (true) {
    TIntFtrH NewFeatures = GenerateRecursiveFeatures(Graph, RetainedFeatures);
    RetainedFeatures = PruneRecursiveFeatures(Graph, Features, NewFeatures,
        SimilarityThreshold);
    if (0 == GetNumFeatures(RetainedFeatures)) {
      break;
    }
    AppendFeatures(Features, RetainedFeatures);
    ++SimilarityThreshold;
    printf("recursion %d: ", SimilarityThreshold);
    printf("current feature number %d\n", GetNumFeatures(Features));
  }
}

void AddLocalFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Features.GetDat(TInt(NI.GetId())).Add(NI.GetInDeg());
  }
}

void AddEgonetFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    int NId = NI.GetId();
    int ArndEdges;
    PUNGraph Egonet = GetEgonet(Graph, NId, ArndEdges);
    Features.GetDat(NId).Add(Egonet->GetEdges());
    Features.GetDat(NId).Add(ArndEdges);
  }
}

TIntFtrH GenerateRecursiveFeatures(const PUNGraph Graph,
    const TIntFtrH& CurrFeatures) {
  const int NumCurrFeatures = GetNumFeatures(CurrFeatures);
  if (0 == NumCurrFeatures) {
    return CurrFeatures;
  }
  TIntFtrH NewFeatures = CreateEmptyFeatures(CurrFeatures);
  for (int i = 0; i < NumCurrFeatures; ++i) {
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
      float Sum = 0;
      for (int j = 0; j < NI.GetInDeg(); ++j) {
        int NbrNId = NI.GetInNId(j);
        Sum += CurrFeatures.GetDat(NbrNId)[i]();
      }
      NewFeatures.GetDat(NI.GetId()).Add(Sum);
      NewFeatures.GetDat(NI.GetId()).Add(0 == NI.GetInDeg()?
          0 : (float(Sum) / NI.GetInDeg()));
    }
  }
  return NewFeatures;
}

TIntFtrH PruneRecursiveFeatures(const PUNGraph Graph, const TIntFtrH& Features,
    const TIntFtrH& NewFeatures, const int SimilarityThreshold) {
  TIntFtrH AllFeatures = CreateEmptyFeatures(Features);
  AppendFeatures(AllFeatures, Features);
  AppendFeatures(AllFeatures, NewFeatures);
  const float BinFraction = 0.5;
  TIntFtrH LogBinFeatures = CalcVerticalLogBinning(AllFeatures, BinFraction);
  PUNGraph FeatureGraph = BuildFeatureGraph(LogBinFeatures,
      SimilarityThreshold);
  return SummarizeConnectedComponents(FeatureGraph, Features, NewFeatures);
}

void AppendFeatures(TIntFtrH& DstFeatures, const TIntFtrH& SrcFeatures,
    const int ColIdx) {
  for (TIntFtrH::TIter HI = SrcFeatures.BegI();
      HI < SrcFeatures.EndI();
      HI++) {
    const TFtr& f = HI.GetDat();
    if (ColIdx >= 0) {
      DstFeatures.GetDat(HI.GetKey()).Add(f[ColIdx]);
    } else {
      for (int i = 0; i < f.Len(); ++i) {
        DstFeatures.GetDat(HI.GetKey()).Add(f[i]);
      }
    }
  }
}

TIntFtrH CalcVerticalLogBinning(const TIntFtrH& Features,
    const float BinFraction) {
  const int NumFeatures = GetNumFeatures(Features);
  TIntFtrH LogBinFeatures = CreateEmptyFeatures(Features);
  for (int i = 0; i < NumFeatures; ++i) {
    TVec<TInt> SortedNId = GetNIdSorted(Features, i);
    AssignBinValue(SortedNId, BinFraction, LogBinFeatures);
  }
  return LogBinFeatures;
}

PUNGraph BuildFeatureGraph(const TIntFtrH& LogBinFeatures,
    const int SimilarityThreshold) {
  PUNGraph FeatureGraph = PUNGraph::New();
  const int NumFeatures = GetNumFeatures(LogBinFeatures);
  for (int i = 0; i < NumFeatures; ++i) {
    FeatureGraph->AddNode(i);
  }
  for (int i = 0; i < NumFeatures; ++i) {
    TFtr IthFeature = GetNthFeature(LogBinFeatures, i);
    for (int j = i + 1; j < NumFeatures; ++j) {
      TFtr JthFeature = GetNthFeature(LogBinFeatures, j);
      if (IsSimilarFeature(IthFeature, JthFeature, SimilarityThreshold) &&
          !FeatureGraph->IsEdge(i, j)) {
        FeatureGraph->AddEdge(i, j);
      }
    }
  }
  return FeatureGraph;
}

TIntFtrH SummarizeConnectedComponents(const PUNGraph FeatureGraph,
    const TIntFtrH& Features, const TIntFtrH& NewFeatures) {
  TCnComV Wcc;
  TSnap::GetWccs(FeatureGraph, Wcc);
  TVec<TInt> RetainedIdx;
  for (int i = 0; i < Wcc.Len(); ++i) {
    RetainedIdx.Add(Wcc[i][0]);
  }
  RetainedIdx.Sort();

  TIntFtrH RetainedFeatures = CreateEmptyFeatures(Features);
  const int StartIdxNewFeatures = GetNumFeatures(Features);
  for (int i = 0; i < RetainedIdx.Len(); ++i) {
    const int IdxNewFeatures = RetainedIdx[i] - StartIdxNewFeatures;
    if (IdxNewFeatures >= 0) {
      AppendFeatures(RetainedFeatures, NewFeatures, IdxNewFeatures);
    }
  }
  return RetainedFeatures;
}

TVec<TInt> GetNIdSorted(const TIntFtrH& Features, const int Idx) {
  THash<TInt, TFlt> F;
  for (TIntFtrH::TIter HI = Features.BegI(); HI < Features.EndI(); HI++) {
    F.AddDat(HI.GetKey(), HI.GetDat()[Idx]);
  }
  F.SortByDat();
  TVec<TInt> SortedNId;
  for (THash<TInt, TFlt>::TIter HI = F.BegI(); HI < F.EndI(); HI++) {
    SortedNId.Add(HI.GetKey());
  }
  return SortedNId;
}

void AssignBinValue(const TVec<TInt>& SortedNId, const float BinFraction,
    TIntFtrH& LogBinFeatures) {
  int NumNodes = LogBinFeatures.Len();
  int NumAssigned = 0;
  int BinValue = 0;
  while (NumAssigned < NumNodes) {
    int NumToAssign = ceil(BinFraction * (NumNodes - NumAssigned));
    for (int i = NumAssigned; i < NumAssigned + NumToAssign; ++i) {
      int NId = SortedNId[i];
      LogBinFeatures.GetDat(NId).Add(BinValue);
    }
    NumAssigned += NumToAssign;
    ++BinValue;
  }
}

bool IsSimilarFeature(const TFtr& F1, const TFtr& F2,
    const int SimilarityThreshold) {
  IAssert(F1.Len() == F2.Len());
  for (int i = 0; i < F1.Len(); ++i) {
    if (abs(F1[i] - F2[i]) > SimilarityThreshold) {
      return false;
    }
  }
  return true;
}

TFltVV ConvertFeatureToMatrix(const TIntFtrH& Features) {
  const int NumNodes = Features.Len();
  const int NumFeatures = GetNumFeatures(Features);
  TFltVV FeaturesMtx(NumNodes, NumFeatures);
  for (int i = 0; i < NumNodes; ++i) {
    for (int j = 0; j < NumFeatures; ++j) {
      FeaturesMtx(i, j) = Features.GetDat(i)[j];
    }
  }
  return FeaturesMtx;
}

void PrintMatrix(const TFltVV& Matrix) {
  int XDim = Matrix.GetXDim();
  int YDim = Matrix.GetYDim();
  printf("[");
  for (int i = 0; i < XDim; ++i) {
    printf("[");
    for (int j = 0; j < YDim; ++j) {
      if (j != 0) {
        printf(" ");
      }
      printf("%f", Matrix(i, j)());
    }
    printf("]\n");
  }
  printf("]\n");
}

TFltVV CreateRandMatrix(const int XDim, const int YDim) {
  srand(time(NULL));
  TFltVV Matrix(XDim, YDim);
  for (int i = 0; i < XDim; ++i) {
    for (int j = 0; j < YDim; ++j) {
      Matrix(i, j) = (double) rand() / RAND_MAX;
    }
  }
  return Matrix;
}

bool FltIsZero(const TFlt f) {
  return TFlt::Abs(f) < TFlt::Eps;
}

void CalcNonNegativeFactorization(const TFltVV& V, const int NumRoles,
    TFltVV& W, TFltVV& H) {
  int NumNodes = V.GetXDim();
  int NumFeatures = V.GetYDim();
  W = CreateRandMatrix(NumNodes, NumRoles);
  H = CreateRandMatrix(NumRoles, NumFeatures);
  TFltVV Product(NumNodes, NumFeatures);
  for (int NumIter = 0; NumIter < 50; ++NumIter) {
    TLinAlg::Multiply(W, H, Product);
    // update W
    printf("first loop\n");
#pragma omp parallel for
    for (int k = 0; k < NumNodes * NumRoles; ++k) {
      int i = k / NumRoles;
      int a = k % NumRoles;
      float SumU = 0;
      for (int u = 0; u < NumFeatures; ++u) {
        if (!FltIsZero(Product(i, u))) {
          SumU += V(i, u) / Product(i, u) * H(a, u);
        }
      }
      W(i, a) *= SumU;
    }
    printf("second loop\n");
#pragma omp parallel for
    for (int k = 0; k < NumNodes * NumRoles; ++k) {
      int i = k / NumRoles;
      int a = k % NumRoles;
      float SumJ = 0;
      for (int j = 0; j < NumNodes; ++j) {
        SumJ += W(j, a);
      }
      if (!FltIsZero(SumJ)) {
        W(i, a) /= SumJ;
      }
    }
    printf("third loop:%d*%d=%d\n", NumRoles, NumFeatures, NumRoles * NumFeatures);
    // update H
#pragma omp parallel for
    for (int k = 0; k < NumRoles * NumFeatures; ++k) {
      int a = k / NumFeatures;
      int u = k % NumFeatures;
      float SumI = 0;
      for (int i = 0; i < NumNodes; ++i) {
        if (!FltIsZero(Product(i, u))) {
          SumI += W(i, a) * V(i, u) / Product(i, u);
        }
      }
      H(a, u) *= SumI;
    }
  }
}

TFlt ComputeDescriptionLength(const TFltVV& V, const TFltVV& G,
    const TFltVV& F) {
  int b = 64;
  int m = b * V.GetYDim() * (V.GetXDim() + F.GetYDim());
  TFlt e = 0;
  TFltVV GF(G.GetXDim(), F.GetYDim());
  TLinAlg::Multiply(G, F, GF);
  for (int i = 0; i < V.GetXDim(); ++i) {
    for (int j = 0; j < V.GetYDim(); ++j) {
      TFlt ValueV = V(i, j);
      TFlt ValueGF = GF(i, j);
      if (FltIsZero(ValueV)) {
        e += ValueGF;
      } else if (!FltIsZero(ValueGF)) {
        e += ValueV * TMath::Log(ValueV / ValueGF) - ValueV + ValueGF;
      }
    }
  }
  return m + e;
}
