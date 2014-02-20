#ifndef __EGONET_H__
#define __EGONET_H__

#include "stdafx.h"

class TUNEgonet;
typedef TPt<TUNEgonet> PUNEgonet;

class TUNEgonet : public TUNGraph {
private:
  int WthnEdges, ArndEdges;
public:
  TUNEgonet(const PUNGraph& Graph, const int CenterNode);
  int GetWthnEdges() { return WthnEdges; }
  int GetArndEdges() { return ArndEdges; }
};

#endif
