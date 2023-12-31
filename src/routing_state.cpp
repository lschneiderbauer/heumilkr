#include "routing_state.h"
#include "tsp_greedy.h"
#include <vector>
#include <stdio.h>
#include <array>
#include <map>
#include <algorithm>

// we create a symmat that is one size smaller than the distances
// (only calculate for sites)
distmat<double> calc_savings(const distmat<double> &d)
{
  distmat<double> savings(d.size() - 1, 0);

  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      savings.acc(i, j) = d.get(0, i + 1) + d.get(0, j + 1) - d.get(i + 1, j + 1);
    }
  }

  return savings;
}

int select_vehicle(const std::vector<int> &vehicle_avail,
                   const std::vector<double> &vehicle_caps,
                   const std::vector<int> &site_vehicle,
                   const std::vector<double> &load,
                   const int a, const int b)
{
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);

    int avail = vehicle_avail[vehicle];
    printf("veh avail %i: %d\n", vehicle, avail);
    if (site_vehicle[a] == vehicle || site_vehicle[b] == vehicle)
    {
      avail += 1;
    }

    if (avail >= 1 && load[a] + load[b] <= vehicle_caps[vehicle])
    {
      return vehicle;
    }
  }

  return -1;
}

// returns Site 1, Site 2, Used vehicle
std::tuple<int, int, int> best_link(const distmat<double> &savings,
                                    const std::vector<double> &load,
                                    const std::vector<int> &site_vehicle,
                                    const std::vector<int> &vehicle_avail,
                                    const std::vector<double> &vehicle_caps,
                                    const udg &graph)
{
  std::tuple<int, int, int> best_link = {-1, -1, -1};
  double max_val = 0;

  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      printf("---\n");
      printf("Link (%d,%d)\n", i, j);
      printf("orig1 %d\n", graph.links_to_origin(i));
      printf("orig2 %d\n", graph.links_to_origin(j));
      printf("selected vehicle %d\n", select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, i, j));
      printf("share cycle %d\n", graph.edges_share_cycle(i, j));

      int selected_vehicle;

      if (graph.links_to_origin(i) &&
          graph.links_to_origin(j) &&
          !graph.edges_share_cycle(i, j) &&
          (selected_vehicle =
               select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, i, j) != -1))
      {

        if (savings.get(i, j) > max_val)
        {
          max_val = savings.get(i, j);
          std::get<0>(best_link) = i;
          std::get<1>(best_link) = j;
          std::get<2>(best_link) = selected_vehicle;
        }
      }
    }
  }

  return best_link;
}

RoutingState::RoutingState(
    // we want a copy of this vector
    const std::vector<double> demand, const distmat<double> &distances,
    const std::vector<int> vehicle_avail, const std::vector<double> &vehicle_caps)
{
  RoutingState::distances = distances;
  RoutingState::vehicle_caps = vehicle_caps;

  RoutingState::graph = udg(demand.size());

  RoutingState::load = demand;
  RoutingState::vehicle_avail = vehicle_avail;
  RoutingState::savings = calc_savings(distances);

  // TODO (default resource ?)
  RoutingState::site_vehicle = std::vector<int>(demand.size());
  for (auto it = site_vehicle.begin(); it != site_vehicle.end(); it++)
  {
    int site = std::distance(site_vehicle.begin(), it);
    *it = 0; // TODO
    RoutingState::vehicle_avail[*it] -= 1;
    RoutingState::site_vehicle[site] = *it;
  }
}

// TRUE if something got relinked,
// FALSE if nothing got relinked (i.e. the procedure stabilized)
bool RoutingState::relink_best()
{
  int a;
  int b;
  int vehicle;
  std::tie(a, b, vehicle) =
      best_link(savings, load,
                site_vehicle, vehicle_avail,
                vehicle_caps, graph);

  printf("bl: (%d,%d)\n", a, b);

  if (!((a == b) && (a == -1)))
  {
    graph.relink_edge(a, b);

    // recalculate load
    for (auto &site : graph.sites_in_cycle(a))
    {
      if (!graph.links_to_origin(site))
      {
        load[site] = 0;
      }
      else
      {
        load[site] = load[a] + load[b];
      }
    }

    // reassign vehicles
    vehicle_avail[site_vehicle[a]] += 1;
    vehicle_avail[site_vehicle[b]] += 1;
    vehicle_avail[vehicle] -= 1;
    site_vehicle[a] = vehicle;
    site_vehicle[b] = vehicle;

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
  std::vector<std::pair<int, int>> cols(cycs.size());

  int run_id = 0;

  for (auto it = cycs.begin(); it != cycs.end(); it++)
  {
    int i = std::distance(cycs.begin(), it);

    // check if we have seen elem before
    if (visited_elements.count(*it) > 0)
    {
      std::vector<int> order = orders[visited_elements[*it]];

      cols[i] = {visited_elements[*it],
                 std::distance(order.begin(),
                               std::find(order.begin(), order.end(), i))};
    }
    else // if we did not see it before
    {
      run_id++;
      visited_elements.insert({*it, run_id});
      std::vector<int> order = tsp_greedy(**it, distances);

      orders.insert({run_id, order});
      cols[i] = {run_id, std::distance(order.begin(),
                                       std::find(order.begin(), order.end(), i))};
    }
  }

  return cols;
}
