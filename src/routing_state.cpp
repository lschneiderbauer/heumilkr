#include "routing_state.h"
#include "tsp_greedy.h"
#include <vector>
#include <numeric>
// #include <stdio.h>
#include <array>
#include <map>
#include <algorithm>
#include <stdexcept>


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

int find_first_free_vehicle(const std::vector<int> &vehicle_avail,
                            const std::vector<double> &vehicle_caps,
                            const double load)
{
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);
    int avail = vehicle_avail[vehicle];

    if (avail >= 1 && load <= vehicle_caps[vehicle])
    {
      return vehicle;
    }
  }

  return -1;
}

int select_initial_vehicle(const std::vector<int> &vehicle_avail,
                           const std::vector<double> &vehicle_caps,
                           const double load)
{

  int vehicle = find_first_free_vehicle(vehicle_avail, vehicle_caps, load);

  if (vehicle != -1) return vehicle;

  // if we are here we did not find any vehicle with fitting capacity:
  // check again, but this time we return the next best vehicle
  // (not caring about capacity)
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);
    int avail = vehicle_avail[vehicle];

    if (avail >= 1)
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
                   const std::vector<int> &site_vehicle,
                   const std::vector<double> &load,
                   const int a, const int b)
{
  for (auto it = vehicle_caps.begin(); it != vehicle_caps.end(); it++)
  {
    int vehicle = std::distance(vehicle_caps.begin(), it);

    int avail = vehicle_avail[vehicle];
    // printf("veh avail %i: %d\n", vehicle, avail);
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
      // printf("---\n");
      // printf("Link (%d,%d)\n", i, j);
      // printf("orig1 %d\n", graph.links_to_origin(i));
      // printf("orig2 %d\n", graph.links_to_origin(j));
      // printf("selected vehicle %d\n", select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, i, j));
      // printf("share cycle %d\n", graph.edges_share_cycle(i, j));

      int selected_vehicle;

      if (graph.links_to_origin(i) && graph.links_to_origin(j) &&
          !graph.edges_share_cycle(i, j) &&
          (selected_vehicle =
               select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, i, j)) != -1)
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
    const std::vector<double> demand, const distmat<double> &distances,
    const std::vector<int> vehicle_avail, const std::vector<double> &vehicle_caps)
{
  routing_state::distances = distances;
  routing_state::vehicle_caps = vehicle_caps;

  routing_state::graph = udg(demand.size());

  routing_state::load = demand;
  routing_state::vehicle_avail = vehicle_avail;
  routing_state::savings = calc_savings(distances);

  // this is potentially a big space waste because most of them will be 0.
  // (but we do it anyways because time is more important than space)
  routing_state::singleton_runs =
      std::vector<std::vector<int>>(
          vehicle_caps.size(), std::vector<int>(demand.size(), 0));

  // first resource assignments
  routing_state::site_vehicle = std::vector<int>(demand.size());
  for (auto it = site_vehicle.begin(); it != site_vehicle.end(); it++)
  {
    int site = std::distance(site_vehicle.begin(), it);

    int vehicle = select_initial_vehicle(routing_state::vehicle_avail,
                                         routing_state::vehicle_caps,
                                         load[site]);
    routing_state::vehicle_avail[vehicle] -= 1;

    while (load[site] > vehicle_caps[vehicle])
    {
      load[site] -= vehicle_caps[vehicle];
      routing_state::vehicle_avail[vehicle] -= 1;
      // add those to the extra list "singleton_runs" which we will not
      // touch until the end.
      routing_state::singleton_runs[vehicle][site] += 1;
      vehicle = select_initial_vehicle(routing_state::vehicle_avail,
                                       routing_state::vehicle_caps,
                                       load[site]);
    }

    // only add the last one to the state
    *it = vehicle;
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
      best_link(savings, load,
                site_vehicle, vehicle_avail,
                vehicle_caps, graph);

  // printf("bl: (%d,%d)\n", a, b);

  if (!((a == b) && (a == -1)))
  {
    graph.relink_edge(a, b);

    double new_load = load[a] + load[b];

    // return two vehicles
    vehicle_avail[site_vehicle[a]] += 1;
    vehicle_avail[site_vehicle[b]] += 1;
    // take one vehicle
    vehicle_avail[vehicle] -= 1;

    // recalculate load
    for (auto &site : graph.sites_in_cycle(a))
    {
      // (since we don't use the intermediate ones anyways)
      // we can as well set all of them to the full load
      load[site] = new_load;
      site_vehicle[site] = vehicle;

      // TODO: these are actually properties that should be assigned
      // once per cycle. we should build that into udg somehow, we
      // would save some ~ linear-time assignments.
      // (constant time from here (actually depends on the demand inputs)
      // times the nodes we are relinking.)
    }

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

  for (auto& cyc : graph.get_cycs())
  {
    int site = *((*cyc).begin());

    // free current vehicle before we look for the next best one
    vehicle_avail[site_vehicle[site]] += 1;

    int vehicle = find_first_free_vehicle(
      vehicle_avail, vehicle_caps, load[site]
    );

    vehicle_avail[vehicle] -= 1;

    if (vehicle != -1 && vehicle != site_vehicle[site])
    {
      for (auto& cyc_site : *cyc)
      {
        site_vehicle[cyc_site] = vehicle;
      }

      changed = true;
    }
  }
  return changed;
}


// 1 - site
// 2 - run
// 3 - order
// 4 - vehicle
// 5 - load
col_types routing_state::runs_as_cols() const
{
  typedef std::shared_ptr<std::unordered_set<int>> T;
  typedef long unsigned int lui;

  std::vector<T> cycs = graph.get_cycs();

  int n_singleton_runs = 0;
  for (auto& v : routing_state::singleton_runs)
    for (auto& n : v)
      n_singleton_runs += n;

  lui col_size = cycs.size() + n_singleton_runs;

  std::map<T, int> visited_elements;
  std::map<int, std::vector<int>> orders;
  col_types cols = {
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<double>(col_size)};

  int run_id = 0;

  lui i;
  for (i = 0; i < cycs.size(); i++)
  {
    std::vector<int> order;
    T cyc = cycs[i];

    std::get<0>(cols)[i] = i;
    std::get<3>(cols)[i] = site_vehicle[i];
    std::get<4>(cols)[i] = load[i];

    // check if we have seen elem before
    if (visited_elements.count(cyc) > 0)
    {
      order = orders[visited_elements[cyc]];

      std::get<1>(cols)[i] = visited_elements[cyc];
    }
    else // if we did not see it before
    {
      visited_elements.insert({cyc, run_id});
      order = tsp_greedy(*cyc, distances);

      orders.insert({run_id, order});

      std::get<1>(cols)[i] = run_id;
      run_id++;
    }

    std::get<2>(cols)[i] = std::distance(order.begin(),
                               std::find(order.begin(), order.end(), i));
  }

  // fill the rest up with singleton runs
  for (lui vehicle = 0; vehicle < singleton_runs.size(); vehicle++)
  {
    for (lui site = 0; site < singleton_runs[vehicle].size(); site++)
    {
      for (int j = 0; j < singleton_runs[vehicle][site]; j++)
      {
        std::get<0>(cols)[i] = site;
        std::get<1>(cols)[i] = run_id;
        std::get<2>(cols)[i] = 0;
        std::get<3>(cols)[i] = vehicle;
        std::get<4>(cols)[i] = routing_state::vehicle_caps[vehicle];
        run_id++;
        i++;
      }
    }
  }

  return cols;
}
