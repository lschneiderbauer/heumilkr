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
  RoutingState(const std::vector<double> demand, const distmat<double> &dist,
               const std::vector<int> &n_res,
               const std::vector<double> &capacities);
  std::vector<std::pair<int, int>> runs_as_cols() const;
  bool relink_best();

private:
  distmat<double> distances;
  // Number and capacity of particular vehicle types
  std::vector<int> n_res;
  std::vector<double> capacities;

  distmat<double> savings;
  udg graph;
  std::vector<double> load;
  std::vector<int> res_ids;

};

#endif
