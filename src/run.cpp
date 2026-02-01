#include "run.h"
#include "tsp_greedy.h"

void run::combine(run &other_run, int new_vehicle, binop_dbl combine_load)
{
    auto other_sites = other_run.sites();
    _sites.insert(other_sites.begin(), other_sites.end());

    // new max load
    double new_max_load = combine_load(this->max_load, other_run.max_load);
    this->max_load = new_max_load;
    other_run.max_load = new_max_load;

    this->vehicle = new_vehicle;
}

std::vector<int> run::ordered_sites(const distmat<double> &distances) const
{
    return tsp_greedy(_sites, distances);
}

std::vector<int> run::ordered_sites(
    const distmat<double> &distances,
    const std::vector<int> &first,
    const std::vector<int> &last) const
{
    if (first.size() == 0 || last.size() == 0)
    {
        return ordered_sites(distances);
    }

    // solve two tsp problems
    std::unordered_set<int> sites_first;
    std::unordered_set<int> sites_last;
    for (const auto site : _sites)
    {
        if (first[site])
        {
            sites_first.insert(site);
        }
        if (last[site])
        {
            sites_last.insert(site);
        }
    }

    std::vector<int> ordered_sites_first = tsp_greedy(sites_first, distances);
    std::vector<int> ordered_sites_last = tsp_greedy(sites_last, distances);

    ordered_sites_first.insert(
        ordered_sites_first.end(),
        ordered_sites_last.rbegin(), ordered_sites_last.rend());

    return ordered_sites_first;
}