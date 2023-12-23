#include "distmat.h"
#include "udg.h"
#include <stdint.h>
#include <unordered_set>
#include <vector>

#ifndef ROUTINGSTATE_H
#define ROUTINGSTATE_H

class RoutingState
{
public:
  RoutingState(std::vector<double> demand, const distmat<double> &dist);
  std::vector<std::pair<int, int>> runs_as_cols() const;
  bool relink_best();

private:
  distmat<double> distances;
  distmat<double> savings;
  udg graph;
  std::vector<double> load;
};

#endif
