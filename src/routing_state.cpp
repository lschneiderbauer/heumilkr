#include "routing_state.h"
#include <vector>
// #include <stdio.h>
#include <map>
#include <algorithm>
#include <set>
#include "union_view.h"


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

// returns Site 1, Site 2, Used vehicle
std::tuple<int, int, int> best_link(const distmat<double> &savings,
                                    Fleet &fleet,
                                    const RunManager &graph)
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
      double saving;

      // primitive benchmarking shows that
      // it seems important for performance that "links_to_origin()" is checked last
      // (probably as it is the most expensive operation)
      if (!graph.edges_share_run(i, j) &&
            ((saving = savings.get(i, j)) > max_val) &&
            graph.links_to_origin(i) && graph.links_to_origin(j))
      {

        fleet.release_vehicle(graph.runs[i]->vehicle);
        fleet.release_vehicle(graph.runs[j]->vehicle);

        selected_vehicle =
          fleet.find_fitting_vehicle(
              union_view(graph.runs[i]->sites(), graph.runs[j]->sites()),
              graph.runs[i]->max_load + graph.runs[j]->max_load,
              false
          );

        fleet.reserve_vehicle(graph.runs[i]->vehicle);
        fleet.reserve_vehicle(graph.runs[j]->vehicle);

        if (selected_vehicle != -1) {
          max_val = saving;
          best_link = {i, j, selected_vehicle};
        }
      }
    }
  }

  return best_link;
}

routing_state::routing_state(
    const std::vector<double> demand, // we want a copy of this vector
    const distmat<double> &distances,
    std::vector<int> vehicle_avail, // we want a copy of this vector
    const std::vector<double> &vehicle_caps,
    const std::vector<std::unordered_set<int>> &restricted_vehicles)
    : distances(distances),
      fleet(vehicle_avail, vehicle_caps, restricted_vehicles),
      graph(demand)
{
  this->savings = calc_savings(distances);

  this->singleton_runs = std::vector<run>();

  // first vehicle assignments (iterate over runs)
  for (auto &run : graph.runs) {
    int site = *(run->sites().begin()); // initial runs have only one site
    int vehicle = fleet.find_fitting_vehicle(run->sites(),
                                             run->max_load,
                                             true);

    fleet.reserve_vehicle(vehicle);

    // special treatment for the case when demand is higher than capacity
    while (run->max_load > fleet.capacity(vehicle))
    {
      run->max_load -= fleet.capacity(vehicle);
      this->singleton_runs.emplace_back(site, fleet.capacity(vehicle), vehicle);

      vehicle = fleet.find_fitting_vehicle(run->sites(),
                                           run->max_load,
                                           true);
      fleet.reserve_vehicle(vehicle);
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
  std::tie(a, b, vehicle) = best_link(savings, fleet, graph);

  // printf("---\n");
  // printf("Best Link (%d,%d)\n", a, b);
  // printf("orig1 %d\n", graph.links_to_origin(a));
  // printf("orig2 %d\n", graph.links_to_origin(b));
  // printf("selected vehicle %d\n", vehicle);
  // printf("share cycle %d\n", graph.edges_share_cycle(a, b));

  if (!((a == b) && (a == -1)))
  {
    // return two vehicles
    fleet.release_vehicle(graph.runs[a]->vehicle);
    fleet.release_vehicle(graph.runs[b]->vehicle);
    fleet.reserve_vehicle(vehicle);

    graph.combine_runs(a, b, vehicle);

    return true;
  }
  else
  {
    return false;
  }
}

void routing_state::opt_vehicles()
{
  // first release all vehicles
  for (auto &run : graph.runs)
  {
    fleet.release_vehicle(run->vehicle);
  }

  // then reassign fitting vehicles
  for (auto &run : graph.runs)
  {
    int vehicle =
      fleet.find_fitting_vehicle(
        run->sites(),
        run->max_load,
        true
      );

    fleet.reserve_vehicle(vehicle);                                          
    run->vehicle = vehicle;
  }
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

  lui col_size = runs.size() + this->singleton_runs.size();

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
      order = cyc->ordered_sites(distances);
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
  for (auto &run : this->singleton_runs)
  {
    int site = *(run.sites().begin());
    std::get<0>(cols)[i] = site + 1;
    std::get<1>(cols)[i] = run_id;
    std::get<2>(cols)[i] = 0;
    std::get<3>(cols)[i] = run.vehicle;
    std::get<4>(cols)[i] = fleet.capacity(run.vehicle);
    std::get<5>(cols)[i] = 2 * distances.get(0, 1 + site);
    run_id++;
    i++;
  }

  return cols;
}
