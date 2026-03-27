#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <memory>
#include <unordered_set>

#include "run.h"
#include "fleet.h"
#include "site.h"
#include "symmat.h"

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

typedef std::shared_ptr<run> RunPtr;

class Router
{
public:
    // creates one singleton runs for each site with the given demand and already assigns
    // vehicles from a fleet
    Router(const std::shared_ptr<std::vector<double>> demand,
           const std::unique_ptr<Distmat> distances,
           std::shared_ptr<Fleet> fleet);

    run create_initial_runs(Site s, double demand, std::shared_ptr<Fleet> fleet,
                            const std::shared_ptr<Distmat> distances);
    // creates a new Router by combining two existing ones
    // note that their fleets have to be identical for that to make sense

    bool relink_best();

    // After we have the final routes, we might still be able to assign
    // better vehicles for each route
    // (we might have released some high-priority vehicles on the way which
    // are now unused)
    bool optimize_vehicles();
    void optimize_runs_order();

    // returns the current runs as column vectors for
    // data frame creation
    tbls runs_as_tbls() const;

private:
    // combines the two runs traversing site a and site b with the new vehicle new_vehicle.
    void combine_runs(RunPtr a, RunPtr b, const VehicleTypeID new_vehicle);

    // is site a directly linked to the origin via its traversing run?
    std::optional<std::tuple<Site, Site>> run_merge_order(const Site a, const Site b) const;

    Distmat calc_savings(const Distmat &d) const;

    std::optional<std::tuple<RunPtr, RunPtr, VehicleTypeID>> best_link() const;

    std::shared_ptr<Fleet> fleet;
    const std::shared_ptr<Distmat> distances;
    const std::shared_ptr<std::vector<double>> demand;

    Distmat savings;
    std::vector<run> fixed_singleton_runs; // those runs are not dynamic, i.e. they won't be changed

    std::unordered_set<RunPtr> runs;
};

#endif
