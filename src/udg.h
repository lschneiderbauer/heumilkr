#include <vector>
#include <memory>
#include <unordered_set>
#include "run.h"

#ifndef UDG_H
#define UDG_H

// undirected graph
class udg
{
public:
  udg(const std::vector<double> demand);
  void combine_runs(const int a, const int b, const int new_vehicle);
  bool links_to_origin(const int a) const;
  bool edges_share_run(const int a, const int b) const;
  // a vector of runs (of length of the sites): each site has a reference to
  // the runs it belongs to (which in turn has all the other references)
  std::vector<std::shared_ptr<run>> runs;

private:
  std::vector<int> sites_relinked;
};


#endif
