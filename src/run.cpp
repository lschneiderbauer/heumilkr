#include "run.h"
#include "tsp_greedy.h"

void run::combine(run &other_run, int new_vehicle)
{
    auto other_sites = other_run.sites();
    _sites.insert(other_sites.begin(), other_sites.end());

    // new max load
    double new_max_load = this->max_load + other_run.max_load;
    this->max_load = new_max_load;
    other_run.max_load = new_max_load;

    this->vehicle = new_vehicle;
}

std::vector<int> run::ordered_sites(const distmat<double> &distances) const
{
    return tsp_greedy(_sites, distances);;
}