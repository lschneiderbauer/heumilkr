#include <cpp11.hpp>
#include <iterator>
#include <vector>
#include "runmanager.h"

using namespace cpp11;

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

tbls cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles,
    std::function<void(RunManager &)> callback = [](RunManager &) {});

[[cpp11::register]]
cpp11::writable::list r_cpp_clarke_wright(
    const std::vector<double> &demand,
    const std::vector<double> &distances,
    const std::vector<int> &n_res,
    const std::vector<double> &capacities,
    const std::vector<int> &restr_sites,
    const std::vector<int> &restr_vehicles)
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

  cpp_clarke_wright(
      demand,
      distances,
      n_res,
      capacities,
      restr_sites,
      restr_vehicles,
      [&steps, &demand](RunManager &runm)
      { steps.push_back(tbls_to_dfs(runm.runs_as_tbls(demand))); });

  return steps;
}
