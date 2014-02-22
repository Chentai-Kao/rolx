#include "egonet.h"

TUNEgonet::TUNEgonet(const PUNGraph& Graph, const int CtrNId) {
  this->AddNode(CtrNId);
  const TUNGraph::TNodeI& CtrNode = Graph->GetNI(CtrNId);
  for (int i = 0; i < CtrNode.GetInDeg(); ++i) {
    this->AddNode(CtrNode.GetInNId(i));
  }
  this->CtrNId = CtrNId;
  this->ArndEdges = 0;
  for (int i = 0; i < CtrNode.GetInDeg(); ++i) {
    int NbrNId = CtrNode.GetInNId(i);
    const TUNGraph::TNodeI& NbrNode = Graph->GetNI(NbrNId);
    for (int j = 0; j < NbrNode.GetInDeg(); ++j) {
      int NbrNbrNId = NbrNode.GetInNId(j);
      if (this->IsNode(NbrNbrNId)) {
        if (!this->IsEdge(NbrNId, NbrNbrNId)) {
          this->AddEdge(NbrNId, NbrNbrNId);
        }
      } else {
        this->ArndEdges++;
      }
    }
    this->WthnEdges = this->GetEdges();
  }
}
