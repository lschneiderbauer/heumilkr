#include "run.h"

// initialize a run with a single site
run::run(int site, double max_load)
  : sites(std::unordered_set<int>{site}),
    max_load(max_load),
    vehicle(-1)
{ }

void run::combine(run &other_run, int new_vehicle)
{
    sites.merge(other_run.sites);

    // new max load
    double new_max_load = run::max_load + other_run.max_load;
    this->max_load = new_max_load;
    other_run.max_load = new_max_load;

    this->vehicle = new_vehicle;
}
