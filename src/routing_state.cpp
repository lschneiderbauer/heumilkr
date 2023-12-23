#include "routing_state.h"
#include "tsp_greedy.h"
#include <vector>
#include <stdio.h>
#include <array>
#include <map>
#include <algorithm>


bool combined_loads_exceed_truck_capacity(const std::vector<double> &load, const int a, const int b)
{
  int capacity = 100; // TODO

  return(load[a] + load[b] > capacity);
}

std::array<int,2> best_link(const distmat<double> &savings,
                             const std::vector<double> &load,
                             const udg &graph)
{
  std::array<int,2> max_idx = {-1, -1};
  double max_val = 0;

  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      printf("---\n");
      printf("Link (%d,%d)\n", i, j);
      printf("orig1 %d\n", graph.links_to_origin(i));
      printf("orig2 %d\n", graph.links_to_origin(j));
      printf("exceed %d\n", combined_loads_exceed_truck_capacity(load, i, j));
      printf("share cycle %d\n", graph.edges_share_cycle(i, j));

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

  return max_idx;
}

// we create a symmat that is one size smaller than the distances
// (only calculate for sites)
distmat<double> calc_savings(const distmat<double> &d)
{
  distmat<double> savings(d.size() - 1, 0);

  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      savings.acc(i, j) = d.get(0, i+1) + d.get(0, j+1) - d.get(i+1, j+1);
    }
  }

  return savings;
}

RoutingState::RoutingState(
    // we want a copy of this vector
    const std::vector<double> demand,
    const distmat<double> &dist)
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
  printf("bl: (%d,%d)\n", cell[0], cell[1]);

  if (!((cell[0] == cell[1]) && (cell[0] == -1)))
  {
    graph.relink_edge(cell[0], cell[1]);

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

    return true;
  }
  else
  {
    return false;
  }
}


std::vector<std::pair<int, int>> RoutingState::runs_as_cols() const
{
  typedef std::shared_ptr<std::unordered_set<int>> T;
  std::vector<std::shared_ptr<std::unordered_set<int>>> cycs = graph.get_cycs();

  std::map<T, int> visited_elements;
  std::map<int, std::vector<int>> orders;
  std::vector<std::pair<int, int>> cols (cycs.size());

  int run_id = 0;

  for (auto it = cycs.begin(); it != cycs.end(); it++) {
    int i = std::distance(cycs.begin(), it);

    // check if we have seen elem before
    if (visited_elements.count(*it) > 0)
    {
      std::vector<int> order = orders[visited_elements[*it]];

      cols[i] = { visited_elements[*it],
                  std::distance(order.begin(),
                    std::find(order.begin(), order.end(), i)) };
    }
    else // if we did not see it before
    {
      run_id++;
      visited_elements.insert({*it, run_id});
      std::vector<int> order = tsp_greedy(**it, distances);

      orders.insert({run_id, order});
      cols[i] = { run_id, std::distance(order.begin(),
                            std::find(order.begin(), order.end(), i) )};
    }
  }

  return cols;
}
