#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <vector>
#include "clarke_wright.h"

namespace nb = nanobind;

using DoubleArray =
    nb::ndarray<const double, nb::ndim<1>, nb::c_contig, nb::device::cpu>;

static std::vector<double> to_vector(const DoubleArray &array)
{
  return std::vector<double>(array.data(), array.data() + array.size());
}

NB_MODULE(_core, m)
{
  m.def(
      "clarke_wright",
      [](const DoubleArray &demand,
         const DoubleArray &distances,
         const std::vector<int> &n_res,
         const std::vector<double> &capacities,
         const std::vector<int> &restr_sites,
         const std::vector<int> &restr_vehicles) -> tbls
      {
        const std::vector<double> demand_vec = to_vector(demand);
        const std::vector<double> distances_vec = to_vector(distances);

        // the solver does not touch Python objects, so let other threads run
        nb::gil_scoped_release release;

        return cpp_clarke_wright(
            demand_vec,
            distances_vec,
            n_res,
            capacities,
            restr_sites,
            restr_vehicles);
      },
      "Runs the solver on validated input. See heumilkr.clarke_wright().");
}
