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
    *it = std::unordered_set { -1 };
  }

  cycs = std::vector<std::shared_ptr<std::unordered_set<int>>>(n_vertices);
  for (auto it = cycs.begin(); it != cycs.end(); it++)
  {
    int i = std::distance(cycs.begin(), it);
    *it = std::make_shared<std::unordered_set<int>>(std::unordered_set { i });
  }
}

void udg::add_edge(const int a, const int b)
{
  adj[a].insert(b);
  adj[b].insert(a);

  (*cycs[a]).merge(*cycs[b]);

  cycs[b] = cycs[a];
  // since all vertices in the cycles are affected,
  // we need to update them all.
  // for (auto &s : cycs[a])
  // {
  //   cycs[s] = cycs[a];
  // }
}

void udg::remove_origin_edge(const int a)
{
  adj[a].erase(-1);

  // in this case we do not have to modify cycs,
  // since severing an origin-edge does not break any existing
  // cycles
}

bool udg::links_to_origin(const int a) const
{
  return(adj[a].find(-1) != adj[a].end());
}

bool udg::links_only_to_origin(const int a) const
{
  return(adj[a].find(-1) != adj[a].end() && adj[a].size() == 1);
}

bool udg::edges_share_cycle(const int a, const int b) const
{
  return((*cycs[a]).find(b) != (*cycs[a]).end());
}

std::unordered_set<std::shared_ptr<std::unordered_set<int>>> udg::con_comps() const
{
  std::unordered_set<std::shared_ptr<std::unordered_set<int>>> unique_cycles;

  for (auto it : cycs) {
    unique_cycles.insert(it);
  }

  return unique_cycles;
}
