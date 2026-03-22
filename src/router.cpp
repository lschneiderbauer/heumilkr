#include "router.h"
#include "union_view.h"
#include "distinct_pairs.h"

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
      demand(demand)
{
  this->savings = calc_savings(*(this->distances));

  fixed_singleton_runs = std::vector<run>();

  runs = std::set<RunPtr>();
  for (size_t i = 0; i < demand->size(); i++)
  {
    runs.insert(std::make_shared<run>(
        create_initial_runs(i, (*demand)[i], fleet, this->distances)));
  }
}

void Router::combine_runs(RunPtr a, RunPtr b, const VehicleTypeID new_vehicle)
{
  assert(a != b);

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

  runs.erase(b);

  a->combine(*b, new_vehicle);
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

std::optional<std::tuple<RunPtr, RunPtr, VehicleTypeID>> Router::best_link() const
{
  std::optional<std::tuple<RunPtr, RunPtr, VehicleTypeID>> best_link(std::nullopt);
  double max_saving = 0;

  // combined load of best configuration
  double combined_load = std::numeric_limits<double>::max();

  for (auto [r1, r2] : distinct_pairs(runs))
  {
    double saving = savings.get(*(r1->sites().rbegin()), *(r2->sites().begin()));

    // TODO: we need to check which direction is better r1->r2, or r2->r1

    if (saving >= max_saving)
    {
      double new_combined_load = r1->combined_max_load(*r2);

      // if the savings are equal, but the new combined load is better, use that one
      // (this can occur when positive and negative demands get combined)
      if (saving > max_saving || new_combined_load < combined_load)
      {
        fleet->release_vehicle(r1->vehicle());
        fleet->release_vehicle(r2->vehicle());

        std::optional<VehicleTypeID> selected_vehicle =
            fleet->find_fitting_vehicle(
                union_view(r1->sites(), r2->sites()),
                new_combined_load,
                false);

        fleet->reserve_vehicle(r1->vehicle());
        fleet->reserve_vehicle(r2->vehicle());

        if (selected_vehicle)
        {
          max_saving = saving;
          combined_load = new_combined_load;
          best_link = {r1, r2, selected_vehicle.value()};
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
  std::optional<std::tuple<RunPtr, RunPtr, VehicleTypeID>> best_link = this->best_link();
  if (!best_link)
  {
    return false;
  }

  RunPtr a;
  RunPtr b;
  VehicleTypeID vehicle;
  std::tie(a, b, vehicle) = best_link.value();

  // return two vehicles
  fleet->release_vehicle(a->vehicle());
  fleet->release_vehicle(b->vehicle());
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
  size_t run_site_col_size = demand->size() + fixed_singleton_runs.size();
  size_t run_col_size = runs.size() + fixed_singleton_runs.size();
  size_t site_col_size = demand->size();

  tbl_run_site run_site_cols = {
      std::vector<int>(run_site_col_size),   // Run ID
      std::vector<int>(run_site_col_size),   // Visited Site
      std::vector<int>(run_site_col_size),   // Order
      std::vector<double>(run_site_col_size) // Departing Load
  };

  tbl_run run_cols = {
      std::vector<int>(run_col_size),    // Run ID
      std::vector<int>(run_col_size),    // Vehicle ID
      std::vector<double>(run_col_size), // Max Load
      std::vector<double>(run_col_size)  // Run distance
  };

  tbl_site site_cols = {
      std::vector<int>(site_col_size),             // Site ID
      std::vector<double>(site_col_size) = *demand // Demand
  };

  int run_id = 0;

  for (auto run : runs)
  {
    std::get<0>(run_cols)[run_id] = run_id;
    std::get<1>(run_cols)[run_id] = run->vehicle();
    std::get<2>(run_cols)[run_id] = run->max_load();
    std::get<3>(run_cols)[run_id] = run->distance();

    int order = 0;
    for (Site site : run->sites())
    {
      std::get<0>(run_site_cols)[site] = run_id;
      std::get<1>(run_site_cols)[site] = site;
      std::get<2>(run_site_cols)[site] = order;

      order++;
    }

    for (const auto [site, load] : run->load_after_visit(*demand))
    {
      std::get<3>(run_site_cols)[site] = load;
    }
    run_id++;
  }

  int i = demand->size();

  // fill the rest up with singleton runs
  for (const auto &run : fixed_singleton_runs)
  {
    Site site = *(run.sites().begin());
    std::get<0>(run_site_cols)[i] = run_id;
    std::get<1>(run_site_cols)[i] = site;
    std::get<2>(run_site_cols)[i] = 0;
    std::get<3>(run_site_cols)[i] = run.max_load();

    std::get<0>(run_cols)[run_id] = run_id;
    std::get<1>(run_cols)[run_id] = run.vehicle();
    std::get<2>(run_cols)[run_id] = run.max_load();
    std::get<3>(run_cols)[run_id] = 2 * distances->get(0, 1 + site);

    run_id++;
    i++;
  }

  for (size_t i = 0; i < demand->size(); i++)
  {
    std::get<0>(site_cols)[i] = i;
  }

  return {site_cols, run_cols, run_site_cols};
}
