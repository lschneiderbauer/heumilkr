#ifndef RUN_H
#define RUN_H

#include <vector>
#include <memory>
#include <unordered_set>
#include "distmat.h"

class run
{
public:
  double max_load;
  int vehicle;

  run(int site, double max_load) // initialize a run with a single site
      : max_load(max_load),
        vehicle(-1),
        _sites(std::unordered_set<int>{site}) {};
  run(int site, double max_load, int vehicle)
      : max_load(max_load),
        vehicle(vehicle),
        _sites(std::unordered_set<int>{site}) {};
  run(std::unordered_set<int> &sites, double max_load, int vehicle)
      : max_load(max_load),
        vehicle(vehicle),
        _sites(sites) {};
  void combine(run &other_run, int new_vehicle);
  const std::unordered_set<int> &sites() const
  {
    return _sites;
  }
  std::vector<int> ordered_sites(const distmat<double> &distances) const;

private:
  std::unordered_set<int> _sites;
};

#endif