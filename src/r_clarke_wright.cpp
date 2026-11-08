#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "clarke_wright.h"
#include "input_error.h"

using namespace cpp11;

// The R side formats the error message, so that it can use the argument names
// and indices of the R user, see abort_input_error().
static cpp11::writable::list input_error_to_list(const InputError &error)
{
  return cpp11::writable::list(
      {"error"_nm = cpp11::writable::list(
           {"code"_nm = cpp11::as_sexp(input_error_name(error.code)),
            "index"_nm = cpp11::as_sexp(double(error.index)),
            "other_index"_nm = cpp11::as_sexp(double(error.other_index)),
            "value"_nm = cpp11::as_sexp(error.value)})});
}

cpp11::writable::list tbls_to_dfs(const tbls &tbls)
{
  tbl_run_site trs = std::get<2>(tbls);
  tbl_site ts = std::get<0>(tbls);
  tbl_run tr = std::get<1>(tbls);

  cpp11::writable::data_frame df_run_site({"run"_nm = as_sexp(std::get<0>(trs)),
                                           "site"_nm = as_sexp(std::get<1>(trs)),
                                           "order"_nm = as_sexp(std::get<2>(trs)),
                                           "load"_nm = as_sexp(std::get<3>(trs))});

  cpp11::writable::data_frame df_run({"run"_nm = as_sexp(std::get<0>(tr)),
                                      "vehicle"_nm = as_sexp(std::get<1>(tr)),
                                      "max_load"_nm = as_sexp(std::get<2>(tr)),
                                      "distance"_nm = as_sexp(std::get<3>(tr))});

  cpp11::writable::data_frame df_site({"site"_nm = as_sexp(std::get<0>(ts)),
                                       "demand"_nm = as_sexp(std::get<1>(ts))});

  cpp11::writable::list dfs({"runs"_nm = df_run,
                             "sites"_nm = df_site,
                             "visits"_nm = df_run_site});

  return dfs;
}

[[cpp11::register]]
cpp11::writable::list r_cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles)
{
  try
  {
    return tbls_to_dfs(
        cpp_clarke_wright(
            demand,
            distances,
            n_res,
            capacities,
            restr_sites,
            restr_vehicles));
  }
  catch (const InputException &e)
  {
    return input_error_to_list(e.error());
  }
}

[[cpp11::register]]
list cpp_clarke_wright_stepwise(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles)
{
  cpp11::writable::list steps;

  try
  {
    cpp_clarke_wright(
        demand,
        distances,
        n_res,
        capacities,
        restr_sites,
        restr_vehicles,
        [&steps](Router &router)
        { steps.push_back(tbls_to_dfs(router.runs_as_tbls())); });
  }
  catch (const InputException &e)
  {
    return input_error_to_list(e.error());
  }

  return steps;
}

[[cpp11::register]]
cpp11::writable::strings cpp_input_error_names()
{
  cpp11::writable::strings names;
  for (const InputErrorCode code : all_input_error_codes())
  {
    names.push_back(input_error_name(code));
  }
  return names;
}
