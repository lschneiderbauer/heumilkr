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
               const std::vector<int> vehicle_avail,
               const std::vector<double> &vehicle_caps);
  std::vector<std::pair<int, int>> runs_as_cols() const;
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

};

#endif
