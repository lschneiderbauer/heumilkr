#include "symmat.h"
#include "udg.h"
#include <stdint.h>
#include <unordered_set>
#include <vector>

#ifndef ROUTINGSTATE_H
#define ROUTINGSTATE_H

class RoutingState
{
public:
  RoutingState(std::vector<int> demand, const symmat<double> &dist);
  std::vector<std::unordered_set<int>> runs() const;
  bool relink_best();

private:
  symmat<double> distances;
  symmat<double> savings;
  udg graph;
  std::vector<int> load;
};

#endif
