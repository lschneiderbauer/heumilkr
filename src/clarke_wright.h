#ifndef CLARKE_WRIGHT_H
#define CLARKE_WRIGHT_H

#include <functional>
#include <vector>
#include "router.h"

// Entry point of the solver, shared by the R and the Python package.
//
// * `demand`: the demand of each of the N sites, finite. Positive values are
//   delivered from the depot, negative ones are picked up.
// * `distances`: the upper triangle of the symmetric (N + 1) x (N + 1) distance
//   matrix row by row, without the diagonal, where the first location is the
//   depot. All finite.
// * `n_res`, `capacities`: the number (not negative) and capacity (positive and
//   finite) of each vehicle type. At least one type is required.
// * `restr_sites`, `restr_vehicles`: pairs of a site and a vehicle type (both
//   starting at 0), meaning that the vehicle type can not approach the site.
//
// Throws an InputException if the input does not satisfy the above, or if there
// are not enough vehicles, see input_error.h.
tbls cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles,
    std::function<void(Router &)> callback = [](Router &) {});

#endif // CLARKE_WRIGHT_H
