#ifndef RUN_H
#define RUN_H

#include "fleet.h"
#include "site.h"
#include "symmat.h"
#include "tsp_greedy.h"
#include <cmath>
#include <list>
#include <map>
#include <memory>
#include <optional>

class run {
public:
  run(Site site, double site_demand, VehicleTypeID vehicle,
      std::shared_ptr<Distmat> distances)
      : _initial_load(site_demand > 0 ? site_demand : 0),
        _final_load(site_demand > 0 ? 0 : -site_demand),
        _max_load(std::abs(site_demand)), _sites(std::list<Site>{site}),
        _vehicle(vehicle), _distances(distances),
        _distance(2 * distances->get(0, 1 + site)) {};

  double combined_max_load(const run &other_run) const;
  std::map<Site, double>
  load_after_visit(const std::vector<double> &demand) const;

  void combine(run &other_run, VehicleTypeID new_vehicle);
  bool reassign_vehicle(Fleet &fleet);
  void optimize_route_order();

  double distance() const { return _distance; }
  const std::list<Site> &sites() const { return _sites; }
  double max_load() const { return _max_load; };
  const VehicleTypeID vehicle() const { return _vehicle; }

private:
  double _initial_load; // load when leaving the origin
  double _final_load;   // load when arriving at the origin
  double _max_load;     // maximal load over the whole run

  std::list<Site> _sites;

  VehicleTypeID _vehicle;

  std::shared_ptr<Distmat> _distances;
  double _distance;
};

#endif
