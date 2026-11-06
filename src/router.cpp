#include "router.h"
#include "union_view.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <vector>
#include <memory>
#include <cassert>
#include <optional>

// creates potentially multiple singleton runs but returns only a single "run".
run Router::create_initial_runs(Site s, double demand, std::shared_ptr<Fleet> fleet,
                                std::shared_ptr<Distmat> distances)
{
  // deliveries (positive demand) and pickups (negative demand) split up the same way
  const double sign = demand < 0 ? -1 : 1;
  double remaining = std::abs(demand);

  while (true)
  {
    const std::optional<VehicleTypeID> vehicle =
        fleet->find_fitting_vehicle(std::list<Site>{s}, remaining, true);

    if (!vehicle)
    {
      throw std::runtime_error(
          "Not enough vehicles available to fulfill all demands trivially."
          " Solver cannot proceed in that case.");
    }

    fleet->reserve_vehicle(*vehicle);
    const double capacity = fleet->capacity(*vehicle);

    if (remaining <= capacity)
    {
      this->run_demand[s] = sign * remaining;
      return run(s, sign * remaining, *vehicle, distances);
    }

    this->fixed_singleton_runs.emplace_back(s, sign * capacity, *vehicle, distances);
    remaining -= capacity;
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
  run_demand = *demand;

  runs.reserve(demand->size());
  for (size_t i = 0; i < demand->size(); i++)
  {
    runs.push_back(std::make_shared<run>(
        create_initial_runs(i, (*demand)[i], fleet, this->distances)));
  }

  best_links.resize(runs.size());
  best_link_stale.assign(runs.size(), true);
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

  const auto b_index = std::distance(runs.begin(), std::find(runs.begin(), runs.end(), b));
  runs.erase(runs.begin() + b_index);
  best_links.erase(best_links.begin() + b_index);
  best_link_stale.erase(best_link_stale.begin() + b_index);

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

std::optional<Router::Link> Router::find_best_link_from(const RunPtr &from) const
{
  std::optional<Link> best_link(std::nullopt);
  double max_saving = 0;

  // combined load of best configuration
  double combined_load = std::numeric_limits<double>::max();

  const Site from_last = *(from->sites().rbegin());

  for (const auto &to : runs)
  {
    if (from != to)
    {
      double saving = savings.get(from_last, *(to->sites().begin()));

      if (saving >= max_saving)
      {
        double new_combined_load = from->combined_max_load(*to);

        // if the savings are equal, but the new combined load is better, use that one
        // (this can occur when positive and negative demands get combined)
        if (saving > max_saving || new_combined_load < combined_load)
        {
          fleet->release_vehicle(from->vehicle());
          fleet->release_vehicle(to->vehicle());

          std::optional<VehicleTypeID> selected_vehicle =
              fleet->find_fitting_vehicle(
                  union_view(from->sites(), to->sites()),
                  new_combined_load,
                  false);

          fleet->reserve_vehicle(from->vehicle());
          fleet->reserve_vehicle(to->vehicle());

          if (selected_vehicle)
          {
            max_saving = saving;
            combined_load = new_combined_load;
            best_link = Link{to, selected_vehicle.value(), saving, new_combined_load};
          }
        }
      }
    }
  }

  return best_link;
}

std::optional<size_t> Router::best_link()
{
  std::optional<size_t> best(std::nullopt);

  for (size_t i = 0; i < runs.size(); i++)
  {
    if (best_link_stale[i])
    {
      best_links[i] = find_best_link_from(runs[i]);
      best_link_stale[i] = false;
    }

    if (best_links[i])
    {
      const Link &link = *best_links[i];

      if (!best ||
          link.saving > best_links[*best]->saving ||
          (link.saving == best_links[*best]->saving &&
           link.combined_load < best_links[*best]->combined_load))
      {
        best = i;
      }
    }
  }

  return best;
}

// TRUE if something got relinked,
// FALSE if nothing got relinked (i.e. the procedure stabilized)
bool Router::relink_best()
{
  const std::optional<size_t> best = this->best_link();
  if (!best)
  {
    return false;
  }

  const RunPtr a = runs[*best];
  const RunPtr b = best_links[*best]->to;
  const VehicleTypeID vehicle = best_links[*best]->vehicle;
  const VehicleTypeID vehicle_a = a->vehicle();
  const VehicleTypeID vehicle_b = b->vehicle();

  const auto any_exhausted = [&]()
  {
    return fleet->is_exhausted(vehicle_a) ||
           fleet->is_exhausted(vehicle_b) ||
           fleet->is_exhausted(vehicle);
  };

  // A vehicle type that is available before and after the merge is available for
  // any link, so only an exhausted type can make other links possible or impossible.
  bool availability_changed = any_exhausted();

  // return two vehicles
  fleet->release_vehicle(vehicle_a);
  fleet->release_vehicle(vehicle_b);
  fleet->reserve_vehicle(vehicle);

  availability_changed = availability_changed || any_exhausted();

  combine_runs(a, b, vehicle);

  /*
  The cached link of a run stays the best one unless it leads to a or b:
  * b does not start a run anymore. This includes the merged run itself, whose
    link led to b, and which now ends at a different site.
  * A link to the merged run has a higher load than the link to a.
  Links to other runs do not change. A link to the merged run can not replace them
  either: it has the same saving as the link to a but can only have a higher load
  and more vehicle restrictions.
  */
  for (size_t i = 0; i < runs.size(); i++)
  {
    if (availability_changed ||
        (best_links[i] && (best_links[i]->to == a || best_links[i]->to == b)))
    {
      best_link_stale[i] = true;
    }
  }

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
  for (const auto &run : runs)
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
  for (const auto &run : runs)
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
      std::vector<double>(run_site_col_size) // Load after visit
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

  for (const auto &run : runs)
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

    for (const auto [site, load] : run->load_after_visit(run_demand))
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
    std::get<3>(run_site_cols)[i] = run.final_load(); // a single site: load after the visit

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
