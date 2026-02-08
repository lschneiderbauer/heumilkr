#include "router.h"
#include "union_view.h"

#include <unordered_set>
#include <map>
#include <algorithm>
#include <iterator>
#include <vector>
#include <memory>
#include <cassert>
#include <optional>

// creates potentially multiple singleton runs but returns only a single "run".
run Router::create_initial_runs(Site s, double demand, std::shared_ptr<Fleet> fleet,
                                std::shared_ptr<Distmat> distances)
{
  VehicleTypeID vehicle;
  try
  {
    vehicle = fleet->find_fitting_vehicle(std::list<Site>{s}, demand, true).value();
  }
  catch (const std::bad_optional_access &e)
  {
    throw std::runtime_error(
        "Not enough vehicles available to fulfill all demands trivially."
        " Solver cannot proceed in that case.");
  }
  fleet->reserve_vehicle(vehicle);
  int capacity = fleet->capacity(vehicle);

  if (demand > capacity)
  {
    this->fixed_singleton_runs.emplace_back(s, capacity, vehicle, distances);
    return (create_initial_runs(s, demand - capacity, fleet, distances));
  }
  else
  {
    return run(s, demand, vehicle, distances);
  }
}

Router::Router(const std::shared_ptr<std::vector<double>> demand,
               const std::unique_ptr<Distmat> distances,
               std::shared_ptr<Fleet> fleet)
    : fleet(fleet),
      distances(std::make_shared<Distmat>(std::move(*distances))),
      demand(demand),
      sites_start(demand->size(), true),
      sites_end(demand->size(), true),
      runs(demand->size())
{
  this->savings = calc_savings(*(this->distances));

  fixed_singleton_runs = std::vector<run>();

  runs.reserve(demand->size()); // just in case
  for (size_t i = 0; i < demand->size(); i++)
  {
    runs[i] = std::make_shared<run>(
        create_initial_runs(i, (*demand)[i], fleet, this->distances));
  }
}

void Router::combine_runs(const Site a, const Site b, const VehicleTypeID new_vehicle)
{
  assert(a != b);
  assert(runs[a] != runs[b]);
  assert(end_of_run(a));
  assert(start_of_run(b));

  /*
  In the original algorithm we are only supposed to attempt to combine vertices that
  are connected to the origin.
  We used to keep track of the whole graph to determine that, but that was
  not necessary if the initial state consists of singleton runs all connected
  to the origin, i.e. the form a run_ptrle (ORIGIN - v - ORIGIN).
  When combining two vertices v and b for the first time, v is adjacent to ORIGIN
  and b. When combined a second time (to another vertex c), v is adjacent to b and c.
  -> A vertex can be combined at most two times, then it will not be connected to
     the origin anymore.
  */

  runs[a]->combine(*runs[b], new_vehicle);

  sites_end[a] = false;
  sites_start[b] = false;

  // all vertices in the runs are affected,
  // we need to reset the pointer of the ones that b pointed
  // to to point to the same run_ptrle
  for (const auto site : runs[a]->sites())
  {
    runs[site] = runs[a];
  }
}

bool Router::end_of_run(const Site a) const
{
  return sites_end[a];
}

bool Router::start_of_run(const Site a) const
{
  return sites_start[a];
}

bool Router::sites_share_run(const Site a, const Site b) const
{
  return (runs[a] == runs[b]);
}

// we create a Distmat that is one size smaller than the distances
// (only calculate for sites)
Distmat Router::calc_savings(const Distmat &d) const
{
  Distmat savings(d.size() - 1, 0);
  for (int i = 1; i < savings.size(); i++)
  {
    for (int j = 0; j < i; j++)
    {
      savings.acc(i, j) = d.get(0, i + 1) + d.get(0, j + 1) - d.get(i + 1, j + 1);
    }
  }

  return savings;
}

std::optional<std::tuple<Site, Site>> Router::run_merge_order(const Site i, const Site j) const
{
  // if we have only attach a singleton run, we attach the new one at the end
  // if its associated to negative demand, and in the beginning, if it's positive
  bool i_is_singleton = end_of_run(i) && start_of_run(i);
  bool j_is_singleton = end_of_run(j) && start_of_run(j);
  if (
      (i_is_singleton && ((*demand)[i] > 0) && start_of_run(j)) ||
      (j_is_singleton && ((*demand)[j] > 0) && end_of_run(i)) ||
      (end_of_run(i) && start_of_run(j)))
  {
    return std::make_tuple(i, j);
  }

  if (
      (i_is_singleton && ((*demand)[i] < 0) && end_of_run(j)) ||
      (j_is_singleton && ((*demand)[j] < 0) && start_of_run(i)) ||
      (end_of_run(j) && start_of_run(i)))
  {
    return std::make_tuple(j, i);
  }

  return std::nullopt;
}

// returns Site 1, Site 2, Used vehicle
std::optional<std::tuple<Site, Site, VehicleTypeID>> Router::best_link() const
{
  std::optional<std::tuple<Site, Site, VehicleTypeID>> best_link(std::nullopt);
  double max_val = 0;

  for (Site i = 1; i < savings.size(); i++)
  {
    for (Site j = 0; j < i; j++)
    {
      // printf("---\n");
      // printf("Link (%d,%d)\n", i, j);
      // printf("orig1 %d\n", runm.links_to_origin(i));
      // printf("orig2 %d\n", runm.links_to_origin(j));
      // printf("selected vehicle %d\n", select_vehicle(vehicle_avail, vehicle_caps, site_vehicle, load, restricted_vehicles, runm, i, j));
      // printf("share run_ptrle %d\n", runm.sites_share_run_ptrle(i, j));

      std::optional<VehicleTypeID> selected_vehicle;
      double saving;

      if (!sites_share_run(i, j) &&
          ((saving = savings.get(i, j)) > max_val) &&
          ((end_of_run(i) && start_of_run(j)) || (end_of_run(j) && start_of_run(i))))
      {
        // we need to decide how to merge runs together
        std::optional<std::tuple<Site, Site>> mo = run_merge_order(i, j);

        if (mo)
        {
          auto [end, start] = mo.value();

          fleet->release_vehicle(runs[end]->vehicle());
          fleet->release_vehicle(runs[start]->vehicle());

          selected_vehicle =
              fleet->find_fitting_vehicle(
                  union_view(runs[end]->sites(), runs[start]->sites()),
                  runs[end]->combined_max_load(*runs[start]),
                  false);

          fleet->reserve_vehicle(runs[end]->vehicle());
          fleet->reserve_vehicle(runs[start]->vehicle());

          if (selected_vehicle)
          {
            max_val = saving;
            best_link = {end, start, selected_vehicle.value()};
          }
        }
      }
    }
  }

  return best_link;
}

// TRUE if something got relinked,
// FALSE if nothing got relinked (i.e. the procedure stabilized)
bool Router::relink_best()
{
  std::optional<std::tuple<Site, Site, VehicleTypeID>> best_link = this->best_link();
  if (!best_link)
  {
    return false;
  }

  Site a;
  Site b;
  VehicleTypeID vehicle;
  std::tie(a, b, vehicle) = best_link.value();

  // return two vehicles
  fleet->release_vehicle(this->runs[a]->vehicle());
  fleet->release_vehicle(this->runs[b]->vehicle());
  fleet->reserve_vehicle(vehicle);

  combine_runs(a, b, vehicle);

  return true;
}

bool Router::optimize_vehicles()
{
  /*
  It might seem more efficient to release all vehicles first and then determine them from
  the ground up.
  However, we run the risk that we cannot reconstruct a vehicle configuration that
  satisfies all requirements (so that we run out of required vehicles).
  -> We simply try one after another, and iterate.
  */

  bool changed = false;

  // then reassign fitting vehicles
  for (auto run : runs)
  {
    if (run->reassign_vehicle(*fleet))
    {
      changed = true;
    }
  }

  return changed;
}

void Router::optimize_runs_order()
{
  for (auto run : runs)
  {
    run->optimize_route_order();
  }
}

tbls Router::runs_as_tbls() const
{
  typedef std::shared_ptr<run> T;

  size_t col_size = runs.size() + fixed_singleton_runs.size();

  std::map<T, int> visited_runs;
  std::map<int, std::list<Site>> orders;
  std::map<int, double> run_dists;

  tbl_run_site run_site_cols = {
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<int>(col_size),
      std::vector<double>(col_size)};

  int run_id = 0;

  // Iterate over sites
  size_t i = 0;
  for (; i < runs.size(); i++)
  {
    std::list<Site> order;
    double run_dist;
    std::shared_ptr<run> run_ptr = runs[i];

    std::get<1>(run_site_cols)[i] = i;

    // check if we have seen run_ptr before
    if (visited_runs.count(run_ptr) > 0)
    {
      order = orders[visited_runs[run_ptr]];
      run_dist = run_dists[visited_runs[run_ptr]];

      std::get<0>(run_site_cols)[i] = visited_runs[run_ptr];
    }
    else // if we did not see it before
    {
      visited_runs.insert({run_ptr, run_id});
      // we reorder each run again (by solving the TSP)
      order = run_ptr->sites();
      run_dist = run_ptr->distance();

      orders.insert({run_id, order});
      run_dists.insert({run_id, run_dist});

      std::get<0>(run_site_cols)[i] = run_id;
      run_id++;
    }

    int norder = std::distance(
        order.begin(),
        std::find(order.begin(), order.end(), i));
    std::get<2>(run_site_cols)[i] = norder;
  }

  // now create the runs table from "visited_runs"
  tbl_run run_cols = {
      std::vector<int>(visited_runs.size() + fixed_singleton_runs.size()),
      std::vector<int>(visited_runs.size() + fixed_singleton_runs.size()),
      std::vector<double>(visited_runs.size() + fixed_singleton_runs.size()),
      std::vector<double>(visited_runs.size() + fixed_singleton_runs.size())};

  for (const auto &[run, run_id] : visited_runs)
  {
    std::get<0>(run_cols)[run_id] = run_id;
    std::get<1>(run_cols)[run_id] = run->vehicle();
    std::get<2>(run_cols)[run_id] = run->max_load();
    std::get<3>(run_cols)[run_id] = run_dists[run_id];

    for (const auto [site, load] : run->load_after_visit(*demand))
    {
      std::get<3>(run_site_cols)[site] = load;
    }
  }

  // fill the rest up with singleton runs
  for (const auto &run : fixed_singleton_runs)
  {
    int site = *(run.sites().begin());
    std::get<0>(run_site_cols)[i] = run_id;
    std::get<1>(run_site_cols)[i] = site;
    std::get<2>(run_site_cols)[i] = 0;
    std::get<3>(run_site_cols)[i] = run.max_load();

    std::get<0>(run_cols)[i] = run_id;
    std::get<1>(run_cols)[i] = run.vehicle();
    std::get<2>(run_cols)[i] = run.max_load();
    std::get<3>(run_cols)[i] = 2 * distances->get(0, 1 + site);

    run_id++;
    i++;
  }

  tbl_site site_cols = {
      std::vector<int>(runs.size()),
      std::vector<double>(runs.size()) = *demand};

  for (i = 0; i < runs.size(); i++)
  {
    std::get<0>(site_cols)[i] = i;
  }

  return {site_cols, run_cols, run_site_cols};
}