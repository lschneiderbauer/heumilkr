#ifndef ROUTINGSTATE_H
#define ROUTINGSTATE_H

#include "distmat.h"
#include "runmanager.h"
#include "fleet.h"
#include "run.h"
#include <unordered_set>
#include <vector>

using col_types = std::tuple<
    std::vector<int>,
    std::vector<int>,
    std::vector<int>,
    std::vector<int>,
    std::vector<double>,
    std::vector<double>>;

class routing_state
{
public:
  routing_state(const std::vector<double> demand,
                const distmat<double> &dist,
                std::vector<int> vehicle_avail,
                const std::vector<double> &vehicle_caps,
                const std::vector<std::unordered_set<int>> &restricted_vehicles);
  col_types runs_as_cols() const;
  bool relink_best();
  void opt_vehicles();

private:
  distmat<double> distances;
  Fleet fleet;

  distmat<double> savings;
  RunManager graph;

  // indexed by vehicle x site
  std::vector<run> singleton_runs;
};

#endif
