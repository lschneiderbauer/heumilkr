#include "routing_state.h"
#include "tsp_greedy.h"
#include <vector>
#include <numeric>
#include <cmath>
// #include <stdio.h>
#include <array>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <limits.h>


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

bool is_vehicle_restricted(const std::unordered_set<int> &restricted_vehicles,
                           int vehicle)
{
  return restricted_vehicles.find(vehicle) != restricted_vehicles.end();
}

int find_first_free_vehicle(const std::vector<int> &vehicle_avail,
                            const std::vector<double> &vehicle_caps,
                            const std::unordered_set<int> &restricted_vehicles,
                            const double max_load)
{
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);
    int avail = vehicle_avail[vehicle];

    if (avail >= 1 &&
        max_load <= vehicle_caps[vehicle] &&
        !is_vehicle_restricted(restricted_vehicles, vehicle))
    {
      return vehicle;
    }
  }

  return -1;
}

int select_initial_vehicle(const std::vector<int> &vehicle_avail,
                           const std::vector<double> &vehicle_caps,
                           const std::unordered_set<int> &restricted_vehicles,
                           const double max_load)
{
  int vehicle = find_first_free_vehicle(vehicle_avail,
                                        vehicle_caps,
                                        restricted_vehicles,
                                        max_load);

  if (vehicle != -1)
    return vehicle;

  // if we are here we did not find any vehicle with fitting capacity:
  // check again, but this time we return the next best vehicle
  // (not caring about capacity)
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);
    int avail = vehicle_avail[vehicle];

    if (avail >= 1 && !is_vehicle_restricted(restricted_vehicles, vehicle))
    {
      return vehicle;
    }
  }

  throw std::runtime_error(
      "Not enough vehicles available to fulfill all demands trivially."
      " Solver cannot proceed in that case.");

  return -1;
}

int select_vehicle(const std::vector<int> &vehicle_avail,
                   const std::vector<double> &vehicle_caps,
                   const std::vector<std::unordered_set<int>> &restricted_vehicles,
                   const udg &graph,
                   const int a, const int b)
{
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);

    int avail = vehicle_avail[vehicle];

    // if vehicle is already in use at this route we artificially increase
    // availability by one, since we can reuse the same vehicle.
    if (graph.runs[a]->vehicle == vehicle || graph.runs[b]->vehicle == vehicle)
    {
      // check for integer overflow:
      // if vehicle number is "infinite" there is no need to increase avail.
      if (avail < INT_MAX) {
        avail += 1;
      }
    }

    bool vehicle_restricted = false;

    // it is not enough to check for vehicle restrictions of the
    // two sites in question: we need to check restrictions of all sites that are
    // already on the same tour as those sites.
    for (auto &site : graph.runs[a]->sites) {
      vehicle_restricted =
        vehicle_restricted ||
          is_vehicle_restricted(restricted_vehicles[site], vehicle);
    }
    for (auto &site : graph.runs[b]->sites) {
      vehicle_restricted =
        vehicle_restricted ||
          is_vehicle_restricted(restricted_vehicles[site], vehicle);
    }

    if (avail >= 1 &&
        graph.runs[a]->max_load + graph.runs[b]->max_load <= vehicle_caps[vehicle] &&
        !vehicle_restricted)
    {
      return vehicle;
    }
  }

  return -1;
}

// returns Site 1, Site 2, Used vehicle
std::tuple<int, int, int> best_link(const distmat<double> &savings,
                                    const std::vector<int> &vehicle_avail,
                                    const std::vector<double> &vehicle_caps,
                                    const std::vector<std::unordered_set<int>> &restricted_vehicles,
                                    const udg &graph)
{
  std::tuple<int, int, int> best_link = {-1, -1, -1};
  double max_val = 0;

  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      // printf("---\n");
      // printf("Link (%d,%d)\n", i, j);
      // printf("orig1 %d\n", graph.links_to_origin(i));
      // printf("orig2 %d\n", graph.links_to_origin(j));
      // printf("selected vehicle %d\n", select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, restricted_vehicles, graph, i, j));
      // printf("share cycle %d\n", graph.edges_share_cycle(i, j));

      int selected_vehicle;

      if (graph.links_to_origin(i) && graph.links_to_origin(j) &&
          !graph.edges_share_cycle(i, j) &&
          (selected_vehicle =
               select_vehicle(vehicle_avail, vehicle_caps,
                              restricted_vehicles, graph, i, j)) != -1)
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

routing_state::routing_state(
    // we want a copy of this vector
    const std::vector<double> demand,
    const distmat<double> &distances,
    const std::vector<int> &vehicle_avail,
    const std::vector<double> &vehicle_caps,
    const std::vector<std::unordered_set<int>> &restricted_vehicles)
    : distances(distances),
      vehicle_avail(vehicle_avail),
      vehicle_caps(vehicle_caps),
      restricted_vehicles(restricted_vehicles),
      graph(demand)
{
  routing_state::savings = calc_savings(distances);

  // this is potentially a big space waste because most of them will be 0.
  // (but we do it anyways because time is more important than space)
  routing_state::singleton_runs =
    std::vector<std::vector<int>>(vehicle_caps.size(), std::vector<int>(demand.size(), 0));

  // first vehicle assignments (iterate over runs)
  for (auto &run : graph.runs) {
    int site = *(run->sites.begin()); // initial runs have only one site
    int vehicle = select_initial_vehicle(routing_state::vehicle_avail,
                                         routing_state::vehicle_caps,
                                         restricted_vehicles[site],
                                         run->max_load);

    routing_state::vehicle_avail[vehicle] -= 1;

    // special treatment for the case when demand is higher than capacity
    while (run->max_load > vehicle_caps[vehicle])
    {
      run->max_load -= vehicle_caps[vehicle];
      routing_state::vehicle_avail[vehicle] -= 1;
      // add those to the extra list "singleton_runs" which we will not
      // touch until the end.
      routing_state::singleton_runs[vehicle][site] += 1;

      vehicle = select_initial_vehicle(routing_state::vehicle_avail,
                                       routing_state::vehicle_caps,
                                       restricted_vehicles[site],
                                       run->max_load);
    }

    // only add the last one to the state
    run->vehicle = vehicle;
  }
}

// TRUE if something got relinked,
// FALSE if nothing got relinked (i.e. the procedure stabilized)
bool routing_state::relink_best()
{
  int a;
  int b;
  int vehicle;
  std::tie(a, b, vehicle) =
      best_link(savings, vehicle_avail,
                vehicle_caps, restricted_vehicles, graph);

  // printf("---\n");
  // printf("Best Link (%d,%d)\n", a, b);
  // printf("orig1 %d\n", graph.links_to_origin(a));
  // printf("orig2 %d\n", graph.links_to_origin(b));
  // printf("selected vehicle %d\n", vehicle);
  // printf("share cycle %d\n", graph.edges_share_cycle(a, b));

  if (!((a == b) && (a == -1)))
  {
    // return two vehicles
    if (vehicle_avail[graph.runs[a]->vehicle] < INT_MAX) {
      vehicle_avail[graph.runs[a]->vehicle] += 1;
    }
    if (vehicle_avail[graph.runs[b]->vehicle] < INT_MAX) {
      vehicle_avail[graph.runs[b]->vehicle] += 1;
    }

    // take one vehicle
    vehicle_avail[vehicle] -= 1;

    graph.combine_runs(a, b, vehicle);

    return true;
  }
  else
  {
    return false;
  }
}

bool routing_state::opt_vehicles()
{
  bool changed = false;

  for (auto &run : graph.runs)
  {
    // free current vehicle before we look for the next best one
    // only free if not "infinite"
    if (vehicle_avail[run->vehicle] < INT_MAX) {
      vehicle_avail[run->vehicle] += 1;
    }

    // unionize vehicle restrictions
    std::unordered_set<int> restr_vehicles;
    for (auto &cyc_site : run->sites)
    {
      // restr_vehicles.merge(routing_state::restricted_vehicles[cyc_sites]);
      restr_vehicles.insert(routing_state::restricted_vehicles[cyc_site].begin(),
                            routing_state::restricted_vehicles[cyc_site].end());
    }

    int vehicle = find_first_free_vehicle(vehicle_avail,
                                          vehicle_caps,
                                          restr_vehicles,
                                          run->max_load);
    vehicle_avail[vehicle] -= 1;

    if (vehicle != -1 && vehicle != run->vehicle)
    {
      run->vehicle = vehicle;

      changed = true;
    }
  }
  return changed;
}

double run_distance(const std::vector<int> ordered_sites,
                    const distmat<double> &d)
{
  auto it = ordered_sites.begin();
  double distance = d.get(0, 1 + *it);

  for (; it < (ordered_sites.end() - 1); it++)
  {
    distance += d.get(1 + *it, 1 + *(it + 1));
  }
  distance += d.get(1 + *it, 0);

  return distance;
}

// 1 - site
// 2 - run
// 3 - order
// 4 - vehicle per run
// 5 - load per run
// 6 - distance per run
col_types routing_state::runs_as_cols() const
{
  typedef std::shared_ptr<run> T;
  typedef long unsigned int lui;

  std::vector<T> runs = graph.runs;

  int n_singleton_runs = 0;
  for (auto &v : routing_state::singleton_runs)
    for (auto &n : v)
      n_singleton_runs += n;

  lui col_size = runs.size() + n_singleton_runs;

  std::map<T, int> visited_elements;
  std::map<int, std::vector<int>> orders;
  std::map<int, double> run_dists;

  col_types cols = {
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<double>(col_size),
      std::vector<double>(col_size)};

  int run_id = 0;

  // Iterate over sites
  lui i;
  for (i = 0; i < runs.size(); i++)
  {
    std::vector<int> order;
    double run_dist;
    T cyc = runs[i];

    std::get<0>(cols)[i] = i + 1;
    std::get<3>(cols)[i] = runs[i]->vehicle;
    std::get<4>(cols)[i] = runs[i]->max_load;

    // check if we have seen cyc before
    if (visited_elements.count(cyc) > 0)
    {
      order = orders[visited_elements[cyc]];
      run_dist = run_dists[visited_elements[cyc]];

      std::get<1>(cols)[i] = visited_elements[cyc];
    }
    else // if we did not see it before
    {
      visited_elements.insert({cyc, run_id});
      // we reorder each run again (by solving the TSP)
      order = tsp_greedy(cyc->sites, distances);
      run_dist = run_distance(order, routing_state::distances);

      orders.insert({run_id, order});
      run_dists.insert({run_id, run_dist});

      std::get<1>(cols)[i] = run_id;
      run_id++;
    }

    std::get<2>(cols)[i] = std::distance(order.begin(),
                                         std::find(order.begin(), order.end(), i));
    std::get<5>(cols)[i] = run_dist;
  }

  // fill the rest up with singleton runs
  for (lui vehicle = 0; vehicle < singleton_runs.size(); vehicle++)
  {
    for (lui site = 0; site < singleton_runs[vehicle].size(); site++)
    {
      for (int j = 0; j < singleton_runs[vehicle][site]; j++)
      {
        std::get<0>(cols)[i] = site + 1;
        std::get<1>(cols)[i] = run_id;
        std::get<2>(cols)[i] = 0;
        std::get<3>(cols)[i] = vehicle;
        std::get<4>(cols)[i] = routing_state::vehicle_caps[vehicle];
        std::get<5>(cols)[i] = 2 * distances.get(0, 1 + site);
        run_id++;
        i++;
      }
    }
  }

  return cols;
}
