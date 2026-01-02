#include "run.h"

void run::combine(run &other_run, int new_vehicle)
{
    sites.merge(other_run.sites);

    // new max load
    double new_max_load = this->max_load + other_run.max_load;
    this->max_load = new_max_load;
    other_run.max_load = new_max_load;

    this->vehicle = new_vehicle;
}
