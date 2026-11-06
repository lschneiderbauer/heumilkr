#ifndef CLARKE_WRIGHT_H
#define CLARKE_WRIGHT_H

#include <functional>
#include <vector>
#include "router.h"

// Entry point of the solver, shared by the R and the Python package.
//
// `distances` holds the upper triangle of the symmetric distance matrix row by
// row, without the diagonal, where the first location is the depot.
tbls cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles,
    std::function<void(Router &)> callback = [](Router &) {});

#endif // CLARKE_WRIGHT_H
