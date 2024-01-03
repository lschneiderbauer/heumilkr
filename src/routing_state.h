#include "distmat.h"
#include "udg.h"
#include <stdint.h>
#include <unordered_set>
#include <vector>

#ifndef ROUTINGSTATE_H
#define ROUTINGSTATE_H

using col_types = std::tuple<
  std::vector<int>,
  std::vector<int>,
  std::vector<int>,
  std::vector<int>,
  std::vector<double>>;

class routing_state
{
public:
  routing_state(const std::vector<double> demand, const distmat<double> &dist,
               const std::vector<int> vehicle_avail,
               const std::vector<double> &vehicle_caps);
  col_types runs_as_cols() const;
  bool relink_best();

private:
  distmat<double> distances;
  // Number and capacity of particular vehicle types
  std::vector<int> vehicle_avail;
  std::vector<double> vehicle_caps;

  distmat<double> savings;
  udg graph;
  std::vector<double> load;
  std::vector<int> site_vehicle;

  // indexed by vehicle x site
  std::vector<std::vector<int>> singleton_runs;
};

#endif
