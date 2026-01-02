#include "udg.h"
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>

udg::udg(const std::vector<double> demand)
  : runs(demand.size()),
    sites_relinked(demand.size(), 0)
{
  runs.reserve(demand.size()); // just in case
  for (size_t i = 0; i < demand.size(); i++)
  {
    runs[i] = std::make_shared<run>(i, demand[i]);
  }
}

void udg::combine_runs(const int a, const int b, const int new_vehicle)
{
  if (a == b) throw std::runtime_error("should not be reachable");
  if (runs[a] == runs[b]) throw std::runtime_error("should not be reachable");
  if (!links_to_origin(a) || !links_to_origin(b))
    throw std::runtime_error("should not be reachable");

  /*
  In the original algorithm we are only supposed to attempt to combine vertices that
  are connected to the origin.
  We used to keep track of the whole graph to determine that, but that was
  not necessary if the initial state consists of singleton runs all connected
  to the origin, i.e. the form a cycle (ORIGIN - v - ORIGIN).
  When combining two vertices v and b for the first time, v is adjacent to ORIGIN
  and b. When combined a second time (to another vertex c), v is adjacent to b and c.
  -> A vertex can be combined at most two times, then it will not be connected to
     the origin anymore.
  */

  sites_relinked[a] += 1;
  sites_relinked[b] += 1;
  runs[a]->combine(*runs[b], new_vehicle);

  // all vertices in the runs are affected,
  // we need to reset the pointer of the ones that b pointed
  // to to point to the same cycle
  for (auto site : runs[a]->sites)
  {
    runs[site] = runs[a];
  }
}

bool udg::links_to_origin(const int a) const
{
  return(sites_relinked[a] < 2);
}

bool udg::edges_share_run(const int a, const int b) const
{
  return (runs[a] == runs[b]);
}
