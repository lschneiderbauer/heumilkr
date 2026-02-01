#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <memory>
#include <unordered_set>

#include "run.h"
#include "fleet.h"

using tbl_run =
    std::tuple<              // Run
        std::vector<int>,    // Run ID
        std::vector<int>,    // Vehicle ID
        std::vector<double>, // Max Load
        std::vector<double>  // Run distance
        >;

using tbl_run_site =
    std::tuple<             // Site-Visit
        std::vector<int>,   // Run ID
        std::vector<int>,   // Visited Site
        std::vector<int>,   // Order
        std::vector<double> // Departing Load
        >;

using tbl_site =
    std::tuple<             // Site
        std::vector<int>,   // Site ID
        std::vector<double> // Demand
        >;

using tbls =
    std::tuple<
        tbl_site,
        tbl_run,
        tbl_run_site>;

class Router
{
public:
  // creates one singleton runs for each site with the given demand and already assigns
  // vehicles from a fleet
  Router(const std::vector<double> &demand,
         std::unique_ptr<distmat<double>> distances,
         std::shared_ptr<Fleet> fleet);

  // creates a new Router by combining two existing ones
  // note that their fleets have to be identical for that to make sense
  Router(const Router &runm1, const Router &runm2,
         const distmat<double> &new_distances,
         const std::vector<int> &site_ind_map1,
         const std::vector<int> &site_ind_map2);

  bool relink_best(binop_dbl combine_load = [](double l1, double l2)
                   { return (l1 + l2); });

  // After we have the final routes, we might still be able to assign
  // better vehicles for each route
  // (we might have released some high-priority vehicles on the way which
  // are now unused)
  bool opt_vehicles();

  // returns the current runs as column vectors for
  // data frame creation
  tbls runs_as_tbls(const std::vector<double> &demand) const;

  std::shared_ptr<Fleet> fleet;
  const std::unique_ptr<distmat<double>> distances;

private:
  // combines the two runs traversing site a and site b with the new vehicle new_vehicle.
  void combine_runs(const int a, const int b, const int new_vehicle, binop_dbl combine_load);

  // is site a directly linked to the origin via its traversing run?
  bool links_to_origin(const int a) const;

  // are the sites a and site b traversed by the same run?
  bool edges_share_run(const int a, const int b) const;

  distmat<double> calc_savings(const distmat<double> &d) const;

  std::tuple<int, int, int> best_link(binop_dbl combine_load) const;

  distmat<double> savings;
  std::vector<int> sites_relinked;
  std::vector<run> fixed_singleton_runs; // those runs are not dynamic, i.e. they won't be changed

  // a vector of runs (of length of the sites): each site has a reference to
  // the runs it belongs to (which in turn has all the other references)
  std::vector<std::shared_ptr<run>> runs;

  bool is_considered(const int site1, const int site2) const;

  // site-indexed: consider only site1-site2 combination for optimization
  // if vectors are not empty
  std::vector<int> consider_optim1;
  std::vector<int> consider_optim2;
};

#endif
