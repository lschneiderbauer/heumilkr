#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>
#include <string>
#include <vector>
#include "clarke_wright.h"
#include "input_error.h"

namespace nb = nanobind;

using DoubleArray =
    nb::ndarray<const double, nb::ndim<1>, nb::c_contig, nb::device::cpu>;

static std::vector<double> to_vector(const DoubleArray &array)
{
  return std::vector<double>(array.data(), array.data() + array.size());
}

NB_MODULE(_core, m)
{
  // Raised with the arguments (code, index, other_index, value) of an
  // InputError. heumilkr.clarke_wright() turns it into a message in Python
  // terms, see input_error.h.
  // The type lives as long as the module, so it is never released.
  PyObject *input_error = PyErr_NewException(
      "heumilkr._core.InputError", PyExc_Exception, nullptr);
  m.attr("InputError") = nb::borrow(input_error);

  nb::register_exception_translator(
      [](const std::exception_ptr &p, void *input_error)
      {
        try
        {
          std::rethrow_exception(p);
        }
        catch (const InputException &e)
        {
          const InputError &error = e.error();
          PyErr_SetObject(
              static_cast<PyObject *>(input_error),
              nb::make_tuple(
                  input_error_name(error.code),
                  error.index,
                  error.other_index,
                  error.value)
                  .ptr());
        }
      },
      input_error);

  m.def(
      "input_error_names",
      []()
      {
        std::vector<std::string> names;
        for (const InputErrorCode code : all_input_error_codes())
        {
          names.push_back(input_error_name(code));
        }
        return names;
      },
      "The codes of all InputErrors.");

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
      "Runs the solver. See heumilkr.clarke_wright().\n\n"
      "Raises InputError on invalid input, see input_error.h.");
}
