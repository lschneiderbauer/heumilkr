#ifndef RUN_H
#define RUN_H

#include <unordered_set>
#include <functional>
#include "distmat.h"

using binop_dbl = std::function<double(const double, const double)>;

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
  void combine(run &other_run, int new_vehicle, binop_dbl combine_load);
  const std::unordered_set<int> &sites() const
  {
    return _sites;
  }
  std::vector<int> ordered_sites(const distmat<double> &distances) const;

  // special version that has predefined order requirements:
  // all "first" sites must come before "last" sites
  std::vector<int> ordered_sites(const distmat<double> &distances,
                                 const std::vector<int> &first,
                                 const std::vector<int> &last) const;

private:
  std::unordered_set<int> _sites;
};

#endif