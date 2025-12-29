#include "udg.h"
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>

udg::udg(const std::vector<double> demand)
  : runs(demand.size()),
    adj(demand.size())
{
  // adj = std::vector<std::unordered_set<int>>(n_vertices);
  for (auto it = adj.begin(); it != adj.end(); it++)
  {
    *it = std::unordered_set<int>{ORIGIN};
  }

  // runs = std::vector<std::shared_ptr<run>>(n_vertices);
  runs.reserve(demand.size()); // just in case
  for (size_t i = 0; i < demand.size(); i++)
  {
    runs[i] = std::make_shared<run>(i, demand[i]);
  }
}

void udg::relink_edge(const int a, const int b)
{
  add_edge(a, b);

  // a vertex can only be connected to either the source alone (ORIGIN),
  // or to another vertex v (ORIGIN, v). If a vertex v is to be connected to another
  // vertex v2, we always have (v, v2), not (ORIGIN, v2).
  if (adj[a].size() > 2)
  {
    adj[a].erase(ORIGIN);
  }
  if (adj[b].size() > 2)
  {
    adj[b].erase(ORIGIN);
  }

}

void udg::combine_runs(const int a, const int b, const int new_vehicle)
{
  if (a == b) throw std::runtime_error("should not be reachable");
  if (runs[a] == runs[b]) throw std::runtime_error("should not be reachable");

  relink_edge(a, b);
  runs[a]->combine(*runs[b], new_vehicle);

  // all vertices in the cycles are affected,
  // we need to reset the pointer of the ones that b pointed
  // to to point to the same cycle
  for (auto site : (*runs[a]).sites)
  {
    runs[site] = runs[a];
  }
}

void udg::add_edge(const int a, const int b)
{
  if (a == b) return;

  adj[a].insert(b);
  adj[b].insert(a);
}

bool udg::links_to_origin(const int a) const
{
  return (adj[a].find(ORIGIN) != adj[a].end());
}

bool udg::edges_share_cycle(const int a, const int b) const
{
  return (runs[a] == runs[b]);
}
