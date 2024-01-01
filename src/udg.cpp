#include "udg.h"
#include <unordered_set>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>

udg::udg(int n_vertices)
{
  adj = std::vector<std::unordered_set<int>>(n_vertices);
  for (auto it = adj.begin(); it != adj.end(); it++)
  {
    *it = std::unordered_set<int>{-1};
  }

  cycs = std::vector<std::shared_ptr<std::unordered_set<int>>>(n_vertices);
  for (auto it = cycs.begin(); it != cycs.end(); it++)
  {
    int i = std::distance(cycs.begin(), it);
    *it = std::make_shared<std::unordered_set<int>>(std::unordered_set<int>{i});
  }
}

void udg::relink_edge(const int a, const int b)
{
  add_edge(a, b);

  // a vertex can only be connected to either the source alone (-1),
  // or to another vertex v (-1, v). If a vertex v is to be connected to another
  // vertex v2, we always have (v, v2), not (-1, v2).
  if (adj[a].size() > 2)
  {
    adj[a].erase(-1);
  }
  if (adj[b].size() > 2)
  {
    adj[b].erase(-1);
  }
}

void udg::add_edge(const int a, const int b)
{
  adj[a].insert(b);
  adj[b].insert(a);

  (*cycs[a]).merge(*cycs[b]);

  // all vertices in the cycles are affected,
  // we need to reset the pointer of the ones that b pointed
  // to to point to the same cycle
  for (auto site : *cycs[a])
  {
    cycs[site] = cycs[a];
  }
}

bool udg::links_to_origin(const int a) const
{
  return (adj[a].find(-1) != adj[a].end());
}

bool udg::edges_share_cycle(const int a, const int b) const
{
  return (cycs[a] == cycs[b]);
}

std::unordered_set<int> udg::sites_in_cycle(const int a) const
{
  return *cycs[a];
}

std::vector<std::shared_ptr<std::unordered_set<int>>> udg::get_cycs() const
{
  return cycs;
}
