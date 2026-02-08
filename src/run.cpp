#include "run.h"
#include <stdexcept>
#include <cassert>

void run::combine(run &other_run, VehicleTypeID new_vehicle)
{
    assert(this->_distances == other_run._distances);

    _sites.splice(_sites.end(), other_run._sites);

    this->_vehicle = new_vehicle;
    this->_max_load = this->combined_max_load(other_run);
    // needs to be after combined_max_load(), as it uses initial_load()
    this->_initial_load += other_run._initial_load;
    this->_final_load += other_run._final_load;

    this->_distance =
        this->_distance + other_run._distance -
        this->_distances->get(0, 1 + *this->_sites.begin()) -
        this->_distances->get(0, 1 + *other_run._sites.rbegin()) +
        this->_distances->get(1 + *this->_sites.rbegin(),
                              1 + *other_run._sites.begin());
}

double run::combined_max_load(const run &other_run) const
{
    // note that this is not symmetric in the order of runs
    return std::max(
        this->_max_load + other_run._initial_load,
        other_run._max_load + this->_final_load);
}

std::map<Site, double> run::load_after_visit(const std::vector<double> &demand) const
{
    std::map<Site, double> loads;

    double cur_load = _initial_load;
    for (Site site : _sites)
    {
        cur_load -= demand[site];
        loads[site] = cur_load;
    }

    return loads;
}

bool run::reassign_vehicle(Fleet &fleet)
{
    VehicleTypeID old_vehicle = this->_vehicle;

    // we are guaranteed to find at least that vehicle once again
    // so the usage of find_fitting_vehicle(...).value() is safe here
    fleet.release_vehicle(old_vehicle);

    VehicleTypeID vehicle =
        fleet.find_fitting_vehicle(
                 this->_sites,
                 this->_max_load,
                 false)
            .value();

    fleet.reserve_vehicle(vehicle);

    if (old_vehicle != vehicle)
    {
        this->_vehicle = vehicle;
        return true;
    }
    return false;
}

void run::optimize_route_order()
{
    std::list<Site> tsp_order;
    double dist;
    std::tie(tsp_order, dist) = tsp_greedy(_sites, *_distances);

    if (dist < this->_distance)
    {
        this->_sites = tsp_order;
        this->_distance = dist;
    }
}