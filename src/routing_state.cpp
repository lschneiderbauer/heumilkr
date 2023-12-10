#include "routing_state.h"
#include <unordered_set>
#include <list>
#include <stdio.h>
#include <array>

bool combined_loads_exceed_truck_capacity(const std::vector<int> &load, const int a, const int b)
{
  int capacity = 100; // TODO

  return(load[a] + load[b] > capacity);
}

std::array<int,2> best_link(const symmat<double> &savings,
                             const std::vector<int> &load,
                             const udg &graph)
{
  std::array<int,2> max_idx = {-1, -1};
  double max_val = 0;
  printf("sav %d\n", savings.size);
  for (int i = 0; i < savings.size; i++)
  {
    for (int j = 0; j < i; j++)
    {
      /*
      printf("orig1 %d\n", graph.links_to_origin(i));
      printf("orig2 %d\n", graph.links_to_origin(j));
      printf("exceed %d\n", !combined_loads_exceed_truck_capacity(load, i, j));
      printf("share cycle %d\n", !graph.edges_share_cycle(i, j));
      */
      if (graph.links_to_origin(i) &&
          graph.links_to_origin(j) &&
          !combined_loads_exceed_truck_capacity(load, i, j) &&
          !graph.edges_share_cycle(i, j))
      {

        if (savings.get(i, j) > max_val)
        {
          max_val = savings.get(i, j);
          max_idx[0] = i;
          max_idx[1] = j;
        }
      }
    }
  }

  return (max_idx);
}

// we create a symmat that is one size smaller than the distances
// (only calculate for sites)
symmat<double> calc_savings(const symmat<double> &d)
{
  symmat<double> savings(d.size - 1, 0);

  for (int i = 0; i < d.size; i++)
  {
    for (int j = 0; j < i; j++)
    {
      savings.acc(i, j) = d.get(0, i+1) + d.get(0, j+1) - d.get(i+1, j+1);
    }
  }

  return (savings);
}

RoutingState::RoutingState(
    // we want a copy of this vector
    const std::vector<int> demand,
    const symmat<double> &dist)
{
  distances = dist;

  graph = udg(demand.size());

  load = demand;
  savings = calc_savings(distances);
}

// TRUE if something got relinked,
// FALSE if nothing got relinked (i.e. the procedure stabilized)
bool RoutingState::relink_best()
{
  std::array<int, 2> cell = best_link(savings, load, graph);
  printf("bl1 %d\n", cell[0]);
  printf("bl2 %d\n", cell[1]);

  if (not ((cell[0] == cell[1]) && (cell[0] == -1)))
  {
    // two edge-possibilities for a vertex:
    // * one -1 (i.e. it is the trivial back-and-forth route):
    //      do not remove that link
    // * -1, c (it non-trivially connects to site c)
    //      remove the (-1,a) link
    graph.add_edge(cell[0], cell[1]);
    if (!graph.links_only_to_origin(cell[0]))
    {
      graph.remove_origin_edge(cell[0]);
    }
    if (!graph.links_only_to_origin(cell[1]))
    {
      graph.remove_origin_edge(cell[1]);
    }

    // recalculate load
    for (auto it = load.begin(); it != load.end(); it++)
    {
      int i = std::distance(load.begin(), it);
      if (!graph.links_to_origin(i)) {
        *it = 0;
      } else {
        *it = load[cell[0]] + load[cell[1]];
      }
    }

    return(true);
  }
  else
  {
    return(false);
  }
}

std::vector<std::unordered_set<int>> RoutingState::runs() const
{
  return(graph.cycles());
}
