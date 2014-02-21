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
  AddNeighborhoodFeatures(Graph, Features);
  AddRecursiveFeatures(Graph, Features);
  return Features;
}

void AddNeighborhoodFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  AddLocalFeatures(Graph, Features);
  AddEgonetFeatures(Graph, Features);
  PrintFeatures(Features);
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
  }
}

void AddLocalFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    Features[TInt(NI.GetId())].Add(NI.GetInDeg());
  }
}

void AddEgonetFeatures(const PUNGraph Graph, TIntFtrH& Features) {
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    int NId = NI.GetId();
    TUNEgonet Egonet(Graph, NId);
    Features[NId].Add(Egonet.GetWthnEdges());
    Features[NId].Add(Egonet.GetArndEdges());
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
        Sum += CurrFeatures[NbrNId][i]();
      }
      NewFeatures[NI.GetId()].Add(Sum);
      NewFeatures[NI.GetId()].Add(0 == NI.GetInDeg()?
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
      DstFeatures[HI.GetKey()].Add(f[ColIdx]);
    } else {
      for (int i = 0; i < f.Len(); ++i) {
        DstFeatures[HI.GetKey()].Add(f[i]);
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
      LogBinFeatures[NId].Add(BinValue);
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
