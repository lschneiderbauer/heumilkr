#include "runmanager.h"
#include <unordered_set>
#include <map>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>
#include "union_view.h"
#include <cassert>

RunManager::RunManager(const std::vector<double> &demand,
                       std::unique_ptr<distmat<double>> distances,
                       std::shared_ptr<Fleet> fleet)
    : fleet(fleet),
      distances(std::move(distances)),
      sites_relinked(demand.size(), 0),
      runs(demand.size())
{
  this->savings = calc_savings(*(this->distances));

  runs.reserve(demand.size()); // just in case
  for (size_t i = 0; i < demand.size(); i++)
  {
    if (demand[i] <= 0)
    {
      throw std::runtime_error("RunManager: demand has to be strictly positive");
    }

    runs[i] = std::make_shared<run>(i, demand[i]);
  }

  fixed_singleton_runs = std::vector<run>();

  // first vehicle assignments (iterate over runs)
  // initial runs have only a single site
  for (auto &run : runs)
  {
    int vehicle = fleet->find_fitting_vehicle(run->sites(),
                                              run->max_load,
                                              true);

    // only reserve vehicles for non-empty runs
    fleet->reserve_vehicle(vehicle);

    // special treatment for the case when demand is higher than capacity
    while (run->max_load > fleet->capacity(vehicle))
    {
      run->max_load -= fleet->capacity(vehicle);
      this->fixed_singleton_runs.emplace_back(
          *(run->sites().begin()), fleet->capacity(vehicle), vehicle);

      vehicle = fleet->find_fitting_vehicle(run->sites(),
                                            run->max_load,
                                            true);

      if (vehicle == -1) {
        throw std::runtime_error(
          "Not enough vehicles available to fulfill all demands trivially."
          " Solver cannot proceed in that case.");
      }

      fleet->reserve_vehicle(vehicle);
    }

    // only add the last one to the state
    run->vehicle = vehicle;
  }
}

int unique_count(union_view<int, std::vector> uv)
{
  std::unordered_set<int> s;
  for (auto it = uv.begin(); it != uv.end(); ++it)
  {
    s.insert(*it);
  }
  return s.size();
}

RunManager::RunManager(const RunManager &runm1, const RunManager &runm2,
                       const distmat<double> &new_distances,
                       const std::vector<int> &site_ind_map1,
                       const std::vector<int> &site_ind_map2) : fleet(runm1.fleet),
                                                                distances(std::make_unique<distmat<double>>(new_distances))
{
  if (runm1.fleet != runm2.fleet)
  {
    throw std::runtime_error("RunManager: cannot combine two RunManagers with different fleets");
  }

  // create inverse maps
  // std::map<int, int> inv_site_ind_map1;
  // for (size_t i = 0; i < site_ind_map1.size(); i++) {
  //   inv_site_ind_map1[site_ind_map1[i]] = i;
  // }
  // std::map<int, int> inv_site_ind_map2;
  // for (size_t i = 0; i < site_ind_map2.size(); i++) {
  //   inv_site_ind_map2[site_ind_map2[i]] = i;
  // }

  // we have to mainly take care of identifying sites of run1 and run2 correctly
  size_t site_size = unique_count(union_view(site_ind_map1, site_ind_map2));

  this->sites_relinked = std::vector<int>(site_size);
  for (size_t i = 0; i < runm1.sites_relinked.size(); i++)
  {
    this->sites_relinked[site_ind_map1[i]] = runm1.sites_relinked[i];
  }
  for (size_t i = 0; i < runm2.sites_relinked.size(); i++)
  {
    this->sites_relinked[site_ind_map2[i]] = runm2.sites_relinked[i];
  }

  this->runs = std::vector<std::shared_ptr<run>>(site_size);
  for (const auto &rptr : runm1.runs)
  {
    std::unordered_set<int> new_sites;
    for (auto site : rptr->sites())
    {
      new_sites.insert(site_ind_map1[site]);
    }
    auto new_run = std::make_shared<run>(new_sites, rptr->max_load, rptr->vehicle);
    for (const auto site : new_run->sites())
    {
      this->runs[site] = new_run;
    }
  }
  for (const auto &rptr : runm2.runs)
  {
    std::unordered_set<int> new_sites;
    for (auto site : rptr->sites())
    {
      new_sites.insert(site_ind_map2[site]);
    }
    auto new_run = std::make_shared<run>(new_sites, rptr->max_load, rptr->vehicle);
    for (const auto site : new_run->sites())
    {
      this->runs[site] = new_run;
    }
  }

  this->fixed_singleton_runs = std::vector<run>();
  for (const auto &srun : runm1.fixed_singleton_runs)
  {
    std::unordered_set<int> new_sites;
    for (auto site : srun.sites())
    {
      new_sites.insert(site_ind_map1[site]);
    }
    this->fixed_singleton_runs.emplace_back(new_sites, srun.max_load, srun.vehicle);
  }
}

void RunManager::combine_runs(const int a, const int b, const int new_vehicle)
{
  if (a == b)
    throw std::runtime_error("should not be reachable");
  if (runs[a] == runs[b])
    throw std::runtime_error("should not be reachable");
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
  for (const auto site : runs[a]->sites())
  {
    runs[site] = runs[a];
  }
}

bool RunManager::links_to_origin(const int a) const
{
  return (sites_relinked[a] < 2);
}

bool RunManager::edges_share_run(const int a, const int b) const
{
  return (runs[a] == runs[b]);
}

// we create a symmat that is one size smaller than the distances
// (only calculate for sites)
distmat<double> RunManager::calc_savings(const distmat<double> &d) const
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
std::tuple<int, int, int> RunManager::best_link() const
{
  std::tuple<int, int, int> best_link = {-1, -1, -1};
  double max_val = 0;

  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      // printf("---\n");
      // printf("Link (%d,%d)\n", i, j);
      // printf("orig1 %d\n", runm.links_to_origin(i));
      // printf("orig2 %d\n", runm.links_to_origin(j));
      // printf("selected vehicle %d\n", select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, restricted_vehicles, runm, i, j));
      // printf("share cycle %d\n", runm.edges_share_cycle(i, j));

      int selected_vehicle;
      double saving;

      // primitive benchmarking shows that
      // it seems important for performance that "links_to_origin()" is checked last
      // (probably as it is the most expensive operation)
      if (!edges_share_run(i, j) &&
          ((saving = savings.get(i, j)) > max_val) &&
          links_to_origin(i) && links_to_origin(j))
      {

        fleet->release_vehicle(runs[i]->vehicle);
        fleet->release_vehicle(runs[j]->vehicle);

        selected_vehicle =
            fleet->find_fitting_vehicle(
                union_view(runs[i]->sites(), runs[j]->sites()),
                runs[i]->max_load + runs[j]->max_load,
                false);

        fleet->reserve_vehicle(runs[i]->vehicle);
        fleet->reserve_vehicle(runs[j]->vehicle);

        if (selected_vehicle != -1)
        {
          max_val = saving;
          best_link = {i, j, selected_vehicle};
        }
      }
    }
  }

  return best_link;
}

// TRUE if something got relinked,
// FALSE if nothing got relinked (i.e. the procedure stabilized)
bool RunManager::relink_best()
{
  int a;
  int b;
  int vehicle;
  std::tie(a, b, vehicle) = best_link();

  // printf("---\n");
  // printf("Best Link (%d,%d)\n", a, b);
  // printf("orig1 %d\n", runm.links_to_origin(a));
  // printf("orig2 %d\n", runm.links_to_origin(b));
  // printf("selected vehicle %d\n", vehicle);
  // printf("share cycle %d\n", runm.edges_share_cycle(a, b));

  if (!((a == b) && (a == -1)))
  {
    // return two vehicles
    fleet->release_vehicle(this->runs[a]->vehicle);
    fleet->release_vehicle(this->runs[b]->vehicle);
    fleet->reserve_vehicle(vehicle);

    combine_runs(a, b, vehicle);

    return true;
  }
  else
  {
    return false;
  }
}

void RunManager::opt_vehicles()
{
  // first release all vehicles
  for (auto &run : this->runs)
  {
    fleet->release_vehicle(run->vehicle);
  }

  // then reassign fitting vehicles
  for (auto &run : runs)
  {
    int vehicle =
        fleet->find_fitting_vehicle(
            run->sites(),
            run->max_load,
            true);

    fleet->reserve_vehicle(vehicle);
    run->vehicle = vehicle;
  }
}

double run_distance(const std::vector<int> ordered_sites,
                    const distmat<double> &d)
{
  auto it = ordered_sites.begin();
  double distance = 2 * d.get(0, 1 + *it);

  for (; it < (ordered_sites.end() - 1); it++)
  {
    distance += d.get(1 + *it, 1 + *(it + 1));
  }

  return distance;
}

// 1 - site
// 2 - run
// 3 - order
// 4 - vehicle per run
// 5 - load per run
// 6 - distance per run
col_types RunManager::runs_as_cols() const
{
  typedef std::shared_ptr<run> T;

  size_t col_size = runs.size() + fixed_singleton_runs.size();

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
  size_t i = 0;
  for (; i < runs.size(); i++)
  {
    std::vector<int> order;
    double run_dist;
    T cyc = runs[i];

    std::get<0>(cols)[i] = i;
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
      order = cyc->ordered_sites(*(this->distances));
      run_dist = run_distance(order, *(this->distances));

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
  for (const auto &run : fixed_singleton_runs)
  {
    int site = *(run.sites().begin());
    std::get<0>(cols)[i] = site;
    std::get<1>(cols)[i] = run_id;
    std::get<2>(cols)[i] = 0;
    std::get<3>(cols)[i] = run.vehicle;
    std::get<4>(cols)[i] = run.max_load;
    std::get<5>(cols)[i] = 2 * distances->get(0, 1 + site);
    run_id++;
    i++;
  }

  return cols;
}
